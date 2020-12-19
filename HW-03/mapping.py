import sys
dic = {}

with open(sys.argv[1], 'r', encoding = 'big5-hkscs') as infile:
    for line in infile.readlines():
        line = line.strip('\n')
        word = line.split(" ")[0]
        zhuyin_set = set()
        zhuyin = line.split(" ")[1]
        zhuyin = zhuyin.split("/")
        for i in zhuyin:
            zhuyin_set.add(i[0])
        for j in list(zhuyin_set):
            if j not in dic.keys():
                dic[j] = [word]
            else:
                dic[j].append(word)
            dic[word] = word

with open(sys.argv[2], 'w', encoding = 'big5-hkscs') as outfile:
    for sth in dic:
        outfile.write(sth + ' ')
        for i in dic[sth]:
            outfile.write(i + ' ')
        outfile.write('\n')
