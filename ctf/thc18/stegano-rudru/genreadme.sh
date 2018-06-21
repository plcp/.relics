cp README.md.template README.md
sed -i "s/title_/$(cat ./src/challenge | head -c 5)/g" README.md
sed -i "s/description_/$(cat ./src/challenge)/g" README.md
