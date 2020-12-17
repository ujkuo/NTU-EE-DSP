import sys
dic = {}

with open(sys.argv[1], 'r', encoding = 'big5-hkscs') as infile:
    for gg in infile.readlines():
        line = gg.strip('\n')
        b = line.split(" ")[0]
        zs = set()
        z = line.split(" ")[1]
        z = z.split("/")
        for omg in z:
            zs.add(omg[0])
        for ohshit in list(zs):
            if ohshit not in dic.keys():
                dic[ohshit] = [b]
            else:
                dic[ohshit].append(b)
            dic[b] = b

with open(sys.argv[2], 'w', encoding = 'big5-hkscs') as outfile:
    for sth in dic:
        outfile.write(sth + ' ')
        for QQ in dic[sth]:
            outfile.write(QQ + ' ')
        outfile.write('\n')
