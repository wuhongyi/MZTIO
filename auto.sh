#!/bin/bash

module load texlive/2019

rm -rf docs/*

make clean

make html
cp -r build/html/* docs/

make latexpdf
mv build/latex/mztio.pdf README.pdf

make clean

touch docs/.nojekyll

cp source/README.md .
