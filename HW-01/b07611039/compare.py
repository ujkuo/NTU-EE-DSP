#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#   Version 
#   Author: ujkuo
#   GitHub: github.com/ujkuo
#   Copyleft (C) 2020 ujkuo All rights reversed.
#

import sys
FILE1 = 'data/test_lbl.txt'
FILE2 = 'result.txt'
pred = []
gt = []
with open(FILE1) as f:
    pred = f.read().split('\n')

with open(FILE2) as f:
    gt = f.read().split('\n')
    gt = [x.split(' ')[0] for x in gt]

count = 0
for i in range(len(pred)):
    if pred[i] == gt[i]:
        count += 1

print('acc = {}'.format(count / len(pred)))