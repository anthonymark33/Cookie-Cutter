#!/bin/sh

# Backup directory
DIR=$(pwd)

# Update submodule
cd ${TRAVIS_BUILD_DIR}/src
git submodule update translations
cd translations

# Generate translation file
lupdate ../Cutter.pro
mv cutter_fr Translations.ts

# Push it
git add Translations.ts
git config user.email "travis@cutter.re"
git config user.name "Travis Auto Build"
git commit -m "Updated translations"
git push "https://${TRAVIS_GITHUB_PUSH}@github.com/radareorg/cutter-translations" master

# Go back to main directory
cd ${DIR}

