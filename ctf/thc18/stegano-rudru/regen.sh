function die()
{
    echo "No internet :("
    cd ..
    rm -rf workdir
    exit
}

mkdir workdir
cd workdir
wget "http://perdu.com"
[ -f index.html ] || echo "ha" > sums
shasum index.html >> sums
echo "4cc21ebecd115325f6c70de1c445a3fd7457843a  index.html" >> sums
cat sums | sort -u | wc -l | grep -q '1' || die

wget "https://twitter.com/denysparsons"
cat denysparsons|grep "400x400"|head -n 1|sed 's/^[^"]*"\([^"]*\)"/\1/' > url
wget "$(cat url)"
mv *400x400.jpeg shrdlu82.jpeg.orig

echo -n "Flag (avec format) ? "
read flag
echo $flag > flag

wget "http://www.musipedia.org/edit.html?&L=0&no_cache=1&tx_detedit_pi1%5Btid%5D=d3afd9019beae27a6709a307a93071ff"
mv edit.html* music
cat music|grep -Eo "[RUD]{6,}"|tr '[:upper:]' '[:lower:]' > riddle
cat music|grep -Eo "lyrics[^P]*Parsons"|grep -Eo "size=.2.>[^<]*</font"|sed \
    's/^.*>//g;s/<.*$//g' > passphrase
cat music|sed 's/<[^>]*>//g'|grep -o "composer/group:[^,]*, .*title:"|sed 's/^[^,]*, //;s/title:$//g' > hint

gpg --batch --passphrase-file ./passphrase -c flag
tar -czvf flag.tar.gz flag.gpg

cp ../embed.py .
python3 embed.py

cd ..
mkdir src 2> /dev/null
cp workdir/shrdlu82.jpeg src
cp workdir/riddle src/challenge
cp workdir/hint src/hint
cp workdir/embedding.json embedding.json

rm -rf workdir
