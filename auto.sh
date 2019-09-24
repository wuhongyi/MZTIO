#!/bin/bash

module load texlive/2019

rm -rf docs/*
cp index.html docs/
touch docs/.nojekyll


cd en/
make clean
make html
make latexpdf
mv build/doctrees/ ../docs/
cp -r build/html/* ../docs/en/
cp build/latex/mztio.pdf ../README_en.pdf
make clean
cd ..

cd zh/
make clean
make html
make latexpdf
cp -r build/html/* ../../docs/zh/
cp build/latex/mztio.pdf ../README_zh.pdf
make clean
cd ../
