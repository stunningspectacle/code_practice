#!/usr/bin/python

import json

nums = [1, 2, 3, 4, 5, 6]
with open("num.json", "w") as f:
    json.dump(nums, f)

with open("num.json") as f:
    load_nums = json.load(f)

print(load_nums)
