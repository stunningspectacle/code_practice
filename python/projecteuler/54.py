#!/usr/bin/python

(   HIGH_CARD,
    ONE_PAIR,
    TWO_PAIRS,
    THREE_OF_A_KIND,
    STRAIGHT,
    FLUSH,
    FULL_HOUSE,
    FOUR_OF_A_KIND,
    STRAIGHT_FLUSH
) = range(9)

card_vals = "23456789TJQKA"
ranks = {}
for i, c in enumerate(card_vals):
    ranks[c] = i

    
def sort_card(vals, reverse=False):
    vals = list(vals)
    for i in xrange(len(vals)):
	for j in xrange(i+1, len(vals)):
	    if reverse:
		if ranks[vals[i]] < ranks[vals[j]]:
		    tmp = vals[i]
		    vals[i] = vals[j]
		    vals[j] = tmp
	    else:
		if ranks[vals[i]] > ranks[vals[j]]:
		    tmp = vals[i]
		    vals[i] = vals[j]
		    vals[j] = tmp
    return "".join(c for c in vals)

def eval_card(cards):
    vals = "".join(card[0] for card in cards)
    flush = (cards[0][1] ==
	     cards[1][1] ==
	     cards[2][1] ==
	     cards[3][1] ==
	     cards[4][1])
    straight = False
    vals = sort_card(vals)
    if vals in card_vals:
	straight = True
    if flush:
	if straight:
	    return (STRAIGHT_FLUSH, vals)
	return (FLUSH, vals)
    if straight:
	return (STRAIGHT, vals)

    from itertools import groupby
    groups = [(len(list(cs)), c) for c, cs in groupby(vals)]
    groups.sort()

    vals = ""
    tail = ""
    for group in groups[::-1]:
	if group[0] > 1:
	    vals += group[1] * group[0]
	else:
	    tail += group[1]
    vals += sort_card(tail, reverse=True)

    if groups[-1][0] == 4:
	return (FOUR_OF_A_KIND, vals)
    if groups[-1][0] == 3:
	if groups[-2][0] == 2:
	    return (FULL_HOUSE, vals)
	return (THREE_OF_A_KIND, vals)
    if groups[-1][0] == 2:
	if groups[-2][0] == 2:
	    return (TWO_PAIRS, vals)
	return (ONE_PAIR, vals)
    return (HIGH_CARD, vals)
    

def prob54():
    poker = open("poker.txt", "r")
    hands = poker.read().split("\n")
    player1_count = 0
    player2_count = 0
    for hand in hands[:-1]:
	hand = hand.split()
	rank1, vals1 = eval_card(hand[:5])
	rank2, vals2 = eval_card(hand[5:])
	if rank1 > rank2:
	    player1_count += 1
	elif rank1 < rank2:
	    player2_count += 1
	else:
	    for i in xrange(5):
		if vals1[i] == vals2[i]:
		    continue
		if ranks[vals1[i]] > ranks[vals2[i]]:
		    player1_count += 1
		    break
		else:
		    player2_count += 1
		    break
    print player1_count
    print player2_count
    
import time
t = time.clock()
prob54()
print time.clock() - t

