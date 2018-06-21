
#
# - Search for "shrdlu 1982" on the internet.
#     - Only one thing named "shrdlu" was published in 1982.
#     - This is a book, named "Best of shrdlu" and written by "Denys Parsons"
#     - Let's found out who Denys is!
#
# - Search for "denys parsons" on the internet.
#     - We find exactly our corrupted image on the first results.
#     - It is from a twitter account with a profile picture.
#     - Let's found out the diff between images!
#

# Prepare ourselves a fresh directory
mkdir workdir
cp ./src/shrdlu82.jpeg ./workdir/
cd workdir

# Retrieve the profile picture
wget "https://twitter.com/denysparsons"
cat denysparsons|grep "400x400"|head -n 1|sed 's/^[^"]*"\([^"]*\)"/\1/' > url
wget "$(cat url)"
mv *400x400.jpeg shrdlu82.jpeg.orig

# Checkout the byte-per-byte "diff"
echo "import sys
with open(sys.argv[1], 'r') as a, open(sys.argv[2], 'r') as b:
    for l, r in zip(a.read().split(), b.read().split()):
        if l != r:
            print('<', l)
            print('>', r)" |\
    python -\
    <(cat shrdlu82.jpeg|xxd -b|grep -Eo "[01]{8}") \
    <(cat shrdlu82.jpeg.orig|xxd -b|grep -Eo "[01]{8}") \
    | grep -v "\---" > diff.bytes

# "$ head diff.bytes" gives us:
#
# < 00010100
# > 00010101
# < 00000101
# > 10000101
# < 00100000
# > 10100000
#
# Seems like each time, only one bit is toggled: let's retrieve information!
#

cat diff.bytes|grep -E "[><]"|xargs|\
    sed 's/< \([01]*\) > \([01]*\)\s\?/print("{:08b}".format(0b\1 ^ 0b\2));/g'\
    | python > result.bytes # terrible, but hey, it works!

# "sort -u result.bytes" gives us:
#
# 00000001
# 00000010
# 00000100
# 00001000
# 00010000
# 00100000
# 01000000
# 10000000
#
# Looks like the data has been encoded:
#  - we have 8 different values (1..8 to be exact).
#  - some kind of "base 8", maybe a "off-by-one" octal?
#  - Let's try to decode this!
#

sed -i 's/00000001/0/g' result.bytes
sed -i 's/00000010/1/g' result.bytes
sed -i 's/00000100/2/g' result.bytes
sed -i 's/00001000/3/g' result.bytes
sed -i 's/00010000/4/g' result.bytes
sed -i 's/00100000/5/g' result.bytes
sed -i 's/01000000/6/g' result.bytes
sed -i 's/10000000/7/g' result.bytes
echo "0" >> result.bytes
echo "0" >> result.bytes
echo "0" >> result.bytes
echo "0" >> result.bytes

cat result.bytes|xargs|\
    sed 's/\([0-9]\) \([0-9]\) \([0-9]\) \([0-9]\)/0o\1\2\3\4,/g' > result.oct
sed -i 's/, [^,]*$/,/g' result.oct
sed -i 's/^0o/[0o/g;s/,$/]/g' result.oct

echo "print(''.join(['{:03x}'.format(d) for d in $(cat result.oct)]))" \
    | python > result.hex

# "cat result.hex|xxd -r -p|file -" tell us that's gzip compressed data! : )

cat result.hex|xxd -r -p > result.gz
gunzip result.gz

# "file result" tell us that's a tar archive! : )

tar -xvf result # gives us "flag.gpg"

# "file flag.gpg" tell us that's symmetrically encrypted data. D:
#
# We haven't used the content of the challenge's description yet:
#
#  "rudrudrudduddduddudrudrudduddd" -- challenge's description
#
# Looking on Denys Parsons's Wikipedia page, we find out that he is quoted for
# a method to represent a melody with a {R, U, D} alphabet :
#
#  https://en.wikipedia.org/wiki/Parsons_code
#
# Wikipedia quotes a website where we can perform Parsons code-based lookups:
#
#  http://www.musipedia.org/about.html
#
# Searching for "rudrudrudduddduddudrudrudduddd" gives us an exact match for:
#
#  Damnation of Faust part 3: Minuet of the will-o'-wisps -- Berlioz, Hector
#
# We try the quoted lyrics as key, trying to guess (sic)

echo -n "La damnation de Faust: Menuet des follets" > key
gpg --batch --passphrase-file key -d flag.gpg > flag

# We have the flag!
shasum ./flag ../flag | grep -Eo "^[^ ]*"|sort -u|wc -l|grep -v -q "1" && \
    echo "Unable to retrieve the flag!" && exit 1

echo -e "\nSuccess!\n\nThe flag is: $(cat ./flag)\n\n"

echo "Press enter to cleanup, ^D to preserve temp files"
read || exit
cd ..
rm -rf workdir
