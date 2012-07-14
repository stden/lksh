#!/bin/bash

rm -f *.{pdf,dvi,log,aux,ps}

for i in *.cpp; do
  name=$(echo "$i" | sed -e 's/\.cpp$//')
  if [ -f "$name" ]; then
    rm -f "$name"
  fi
done

for i in *.src; do
  name="$(cat "$i" | grep 'output' | sed -e 's/^output *= *//').tex"
  if [ -f "$name" ]; then
    rm -f "$name"
  fi
done

