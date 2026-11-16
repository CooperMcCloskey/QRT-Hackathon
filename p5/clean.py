import re,sys
def detok(s):
    s=s.replace(' @-@ ','-').replace(' @,@ ',',').replace(' @.@ ','.')
    s=re.sub(r' ([,.;:!?%)\]])',r'\1',s)
    s=re.sub(r'([(\[$]) ',r'\1',s)
    s=re.sub(r" '(s|t|re|ve|d|ll|m)\b",r"'\1",s)
    s=re.sub(r" n't","n't",s)
    s=re.sub(r'" ([^"]*?) "',r'"\1"',s)
    s=re.sub(r'\s+',' ',s).strip()
    return s
def paras(path):
    out=[]
    for line in open(path,encoding='utf-8'):
        line=line.strip()
        if not line or line.startswith('='): continue
        line=line.replace('<unk>','')
        out.append(detok(line))
    return out
