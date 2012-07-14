#!/bin/sh

if [ "$1" == "" ] ; then
  echo "This utility automatically generates statements from a list of problems (see generic.src)"
  echo "Usage: day_make.sh <source_file> [<template-file>]"
  exit 1
fi

templateFile=$2
if [ "$2" == "" ]; then
  templateFile="template.tex"
fi

echo "compile..."
  g++ -O2 -Wall -o statements_generate statements_generate.cpp || exit 1

sourceFile=$1
namePrefix=`./statements_generate $sourceFile $templateFile`

latex $namePrefix.tex || exit 1
latex $namePrefix.tex || exit 1
dvips $namePrefix.dvi || exit 1
ps2pdf $namePrefix.ps || exit 1 
#dvipdfm -p a4 $namePrefix.dvi || exit 1
#dvips -t landscape $namePrefix.dvi || exit 1
#dvipdfm -p a4 -l $namePrefix.dvi || exit 1

echo "see “$namePrefix.pdf”"

