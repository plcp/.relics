import mpmath as mp

import scipy
import scipy.optimize

"""Fast discrete Gaussian noise generation for LWE-related cryptography

   Reference:
      https://github.com/quarkslab/NFLlib/blob/master/include/nfl/prng/FastGaussianNoise.hpp

   We have rewritten the aforementioned implementation in python for didactic
   purpose. See the tests used to validate our implementation in the same
   directory and further references at the end of this file.

   The comments in this file are intentionally verbose and are directed to
   uninformed readers"""

class gaussian:
    """Fast discrete Gaussian noise generator

       >>> generator = gaussian(sigma, security, nb_samples, center)
       >>> noise = g.get_noise(128)

       Generate discrete Gaussian noise suitable for cryptographic usage (i.e.
       guarantee "security" bits of security when attempting to generate a
       vector of "nb_samples" samples)"""

    lookup_depth = 2
    lookup_size = 16

    def __init__(self, sigma, security, nb_samples, center=0):
        """Fast discrete Gaussian noise generator

           Where:
            - sigma:        standard deviation              (ex: 20)
            - security:     bits-of-security requirement    (ex: 128)
            - nb_samples:   nb. of noise samples required   (ex: 8192)
            - center:       center of the distribution      (ex: 0.0)

           See main class docstring for further details"""

        self.sigma = sigma
        self.security = security
        self.nb_samples = nb_samples

        self.center = mp.mpf(center)
        self.rounded_center = mp.nint(center)

        self.reset()

    def _estimate_bound(self, k_tail):
        """Estimate the "bound" for a good approx. of the Gaussian dist

           We approximate a Gaussian distribution (centered around "center")
           by only issuing points "point" verifying the following property:

                norm(point - center) < bound * sigma

           Were "bound" is the parameter estimated by this function and "sigma"
           the standard deviation of the Gaussian distribution.

           As we will use this Gaussian noise for cryptographic purposes, we
           need to guarantee the security parameter (see "self.__init__").

           We fit the constraints (picked from the paper quoted at the end of
           this file and needed to generate vectors of "nb_samples" samples
           securely) as a find-the-zero problem (we use the Newton-Raphson
           method)."""

        initial_guess = 1 + 2 * k_tail * mp.log(2)

        def f(x):
            return (x * x) - (2 * mp.log(x)) - initial_guess

        def df(x):
            return (2 * x) - (2 / x)

        guess = scipy.optimize.newton(f, initial_guess, df, tol=1e-4)
        if abs(f(guess)) > 1e-4 or f(guess) < 0:
            raise RuntimeError("Unable to find a correct tail bound value.")
        return mp.mpf(guess)

    def reset(self):
        """Reset the generator and its lookup tables for fast noise generation

           These attributes need to be set beforehand: (see self.__init__)
            - self.sigma
            - self.security
            - self.nb_samples
            - self.center

           The following methods will be called:
            - self._estimate_bound()
            - self.compute_barriers
            - self.lookup_table

           These attributes will be initialized:
            - self.tail_bound       ("secure" bound for approximate noise gen.)
            - self.word_precision   (precision of precomputed barriers)
            - self.bit_precision
            - ...                   (attributes set by self.compute_barriers)
            - ...                   (attributes set by self.lookup_table)

           The precision used by mpmath will be set to self.bit_precision."""

        # Estimate bound
        k_tail = self.security + 1 + mp.ceil(mp.log(self.nb_samples, b=2))
        self.tail_bound = self._estimate_bound(k_tail)

        # Initialize internal precision
        epsi = k_tail + mp.log(2 * self.tail_bound * self.sigma)
        self.word_precision = int(mp.ceil(mp.ceil(epsi) / self.lookup_size))
        self.bit_precision = self.word_precision * self.lookup_size

        # Compute barriers
        self.compute_barriers()

        # Build lookup tables (used during noise generation)
        self.build_lookup()

    def compute_barriers(self):
        """Pre-compute the "barriers" used to build the lookup tables

           We pre-compute probabilities at discrete points around the
           center of the distribution. These "barriers" will later be used
           in the Knuth-Yao algorithm to generate a "good-enough" noise.

           We implement the Knuth-Yao algorithm here as a binary tree flattened
           as a couple of lookup table, hence we use the barriers pre-computed
           here to build these lookup tables.

           For alternative implementation of the Knuth-Yao algorithm, see:
                https://github.com/jnortiz/Gaussian-Sampling/blob/step_back/Samplers.cpp#L140

           Also see self.build_lookup where lookup tables are build"""

        # (set precision)
        mp.mp.prec = self.bit_precision

        # Pre-compute 1 / (2 * sigma**2) for further usage
        self.isigma = mp.fdiv(1, mp.fmul(2, mp.power(self.sigma, 2)))

        # Number of barriers (cf. below)
        self.nb_barriers = 1 + 2 * int(mp.ceil(self.tail_bound * self.sigma))
        assert self.nb_barriers < 2**self.lookup_size

        # We compute non-normalized barriers as follows:
        #
        #   bound = mp.ceil(self.tail_bound * self.sigma)
        #   for i in range(-bound, bound + 1):
        #       point = (i + self.rounded_center)
        #       delta = (point - self.center)
        #       final = (delta**2) / (2 * sigma**2)
        #       barrier[i] = mp.exp(-final)         # Gaussian distribution
        #
        # See 4.2 (page 11) on the paper quoted at the end of this file

        def _compute(barrier):
            point = mp.fadd(barrier, self.rounded_center)
            delta = mp.fsub(point, self.center)
            final = mp.fmul(mp.power(delta, 2), self.isigma)
            return mp.exp(mp.fneg(final))

        # Compute non-normalized barriers
        bound = int(self.nb_barriers - 1) // 2
        barriers = []
        for idx, i in enumerate(range(-bound, bound + 1)):
            barrier = _compute(i)
            if idx == 0:
                barriers.append(barrier)
            else:
                barrier = mp.fadd(barrier, barriers[idx - 1])
                barriers.append(barrier)

        # Retrieve the sum of the probabilities
        psum = mp.mpf(barriers[-1])

        # We will normalize barriers by the following factor:
        #   psum = 1 / psum
        #   psum = (2**self.bit_precision - 1) * psum
        #
        psum = mp.fdiv(mp.fsub(mp.power(2, self.bit_precision), 1), psum)

        # Normalize the barriers
        for idx, barrier in enumerate(barriers):
            barriers[idx] = mp.fmul(barrier, psum)

        # "Export" the barriers to later build the lookup tables:
        #   - We represent barriers on "word_precision" words of "lookup_size"
        #   - Hence, it starts with many zeroes followed by the barrier
        #
        wbarriers = []
        for barrier in barriers:
            # Aligned mantissa
            man = int(barrier.man * 2**(barrier.exp % self.lookup_size))

            # Bytes representation
            wratio = int(mp.ceil(self.lookup_size / 8)) # (byte to word ratio)
            man = man.to_bytes(
                (self.word_precision + 1) * wratio, byteorder='big')

            # Squeeze into words
            man = [int.from_bytes(man[i * wratio:(i + 1) * wratio],
                byteorder='big') for i in range(self.word_precision + 1)]

            # Add the right amount of zeroes
            if man[0] == 0:
                man = man[1:]  # if needed, remove extra zero
            zwidth = int(mp.ceil(mp.log(barrier, 2) / self.lookup_size))
            man = [0] * (self.word_precision - zwidth) + man

            # Truncate
            wbarrier = man[:self.word_precision]
            assert len(wbarrier) == self.word_precision

            # Append the others
            wbarriers.append(wbarrier)

        self.barriers = wbarriers

    def build_lookup(self):
        # (set precision)
        mp.mp.prec = self.bit_precision
        bound = int(self.nb_barriers - 1) // 2

        # Initialization
        lookup_nsize = 2**self.lookup_size
        lookup_value = self.rounded_center - bound
        self.lookup_table = [None] * lookup_nsize
        self.lookup_matrix = [None] * lookup_nsize

        # Indexes
        barrier_idx = 0
        row_idx = 0 # First level of lookup depth
        col_idx = 0 # Second level of lookup depth
        lookup_table_flags = 0
        lookup_matrix_flags = 0

        # Lookup element (for internal usage)
        class _lookup_item:
            def __init__(self, value=None, flag=False):
                """Entry of a lookup table

                   Where:
                    - value:    value (barrier[0]) hold by the entry
                    - flag:     Is this entry placed "on a barrier" ?"""

                self.outcomes = []
                self.value = value
                self.flag = flag

        # Building the first level of the lookup table
        #
        # While:
        #  - we stay in the range of the precomputed barriers (probabilities)
        #  - we stay in the range of the first level of the lookup table
        #
        while (True
            and lookup_value <= self.rounded_center + bound
            and row_idx < lookup_nsize):

            # Handle the "simple" cases (until we met a barrier)
            while (True
                and row_idx < self.barriers[barrier_idx][0]
                and row_idx < lookup_nsize):

                    # (handle uninitialized entries)
                    if self.lookup_table[row_idx] is None:
                        self.lookup_table[row_idx] = _lookup_item()

                    # Set table entry
                    self.lookup_table[row_idx] = lookup_value
                    row_idx += 1

            # (handle uninitialized entries)
            if self.lookup_table[row_idx] is None:
                self.lookup_table[row_idx] = _lookup_item()

            # Flag the entry (as an "edge" on a barrier)
            self.lookup_table[row_idx].value = lookup_value;
            self.lookup_table[row_idx].flag = True;
            lookup_table_flags += 1

            # Now populate the second dimension of the lookup table
            col_idx = 0
            self.lookup_matrix[row_idx] = [None] * lookup_nsize

            # Here we are, building the second dimension
            while col_idx < lookup_nsize:

                # Handle "simple" cases
                if (False
                    or row_idx < self.barriers[barrier_idx][0]
                    or col_idx < self.barriers[barrier_idx][1]):

                    # (handle uninitialized entries)
                    if self.lookup_matrix[row_idx][col_idx] is None:
                        self.lookup_matrix[row_idx][col_idx] = _lookup_item()

                    # Set the entry
                    self.lookup_matrix[row_idx][col_idx].value = lookup_value
                    col_idx += 1

                    # No more branches here, skip to the next iteration
                    continue

                # If we are **not** on a barrier, skip to the next iteration
                if not (True
                        and row_idx == self.barriers[barrier_idx][0]
                        and col_idx == self.barriers[barrier_idx][1]):
                    col_idx += 1
                    continue

                # (handle uninitialized entries)
                if self.lookup_matrix[row_idx][col_idx] is None:
                    self.lookup_matrix[row_idx][col_idx] = _lookup_item()

                # Flag the entry (as an "edge" on a barrier)
                self.lookup_matrix[row_idx][col_idx].value = lookup_value
                self.lookup_matrix[row_idx][col_idx].flag = True
                lookup_matrix_flags += 1

                # Add the active barrier to the possible outcomes here
                self.lookup_matrix[row_idx][col_idx].outcomes.append(
                    barrier_idx)
                lookup_value += 1 # Now, use the next active barrier
                barrier_idx += 1

                # Continue to dereference until all possible barriers are added
                while (True
                    and barrier_idx < self.nb_barriers
                    and row_idx == self.barriers[barrier_idx][0]
                    and col_idx == self.barriers[barrier_idx][1]):

                    # Add the active barrier to the possible outcomes here
                    self.lookup_matrix[row_idx][col_idx].outcomes.append(
                        barrier_idx)
                    lookup_value += 1 # Now, use the next active barrier
                    barrier_idx += 1

            # Don't forget to go the next column
            # (as we are building the second dimension of the matrix here)
            #
            col_idx += 1

        # Don't forget to go to the next row
        # (as we are building the first dimension of the matrix here)
        #
        row_idx += 1

    def getNoise(self, nb_bytes):
        # https://github.com/jnortiz/Gaussian-Sampling/blob/step_back/Samplers.cpp#L79

        raise NotImplementedError

# For a good overview (see introduction):
#
# DOI 10.1007/978-3-662-43414-7_20:
#   Discrete Ziggurat: A Time-Memory Trade-off for Sampling from a Gaussian
#   Distribution over the Integers
#
# Note: it refers to the paper quoted below as [GD12]
# See https://eprint.iacr.org/2013/510.pdf

# Bibliography
# ------------
#
# DOI 10.1007/s00200-014-0218-3:
#   Sampling from discrete gaussians for lattice-based cryptography on
#   a constrained device (Nagarjun C. Dwarakanath, Steven Galbraith)
#
# See https://www.math.auckland.ac.nz/~sgal018/gen-gaussians.pdf
