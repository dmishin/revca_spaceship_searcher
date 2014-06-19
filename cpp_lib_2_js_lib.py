#!/usr/bin/env python
import json
import sys

def parse_rle(rle_string, put_cell):
    """based on the CoffeScritp code"""
    x = 0
    y = 0
    curCount = 0
    for i in range(0, len(rle_string)):
        c = rle_string[i]
        if "0" <= c and c <= "9":
            curCount = curCount * 10 + int(c)
        else:
            count = max(curCount, 1)
            curCount = 0
            if c == "b":
                x += count
            elif c == "$":
                y += count
                x = 0
            elif c == "o":
                for j in range(0,count):
                    put_cell(x, y)
                    x+=1
            else:
                raise ValueError( "Unexpected character '%s' at position %d"%(c, i))

def rle2cell_list(rle, dx=0, dy=0):
    cells = []
    parse_rle(rle, lambda x,y: cells.append((x+dx,y+dy)))
    return cells



def convert( data ):
    for record in data["catalog"]:
        result = {"analysed_generations": 4000, 
                  "dx":record["offset"][0],
                  "dy":record["offset"][1],
                  "period": record["period"],
                  "cells": rle2cell_list(record["pattern"])}
        orec = {"result":result,
                "count": record["count"],
                "key": record["pattern"] }
        yield orec


data = json.load( sys.stdin )
print( "version:", data["version"], file=sys.stderr)
json.dump( list(convert(data)), sys.stdout )
