#!/usr/bin/env python3
"""gen.py: make Document-Lego tests. usage: gen.py <seed> <pages> <outprefix> [corpus]"""
import sys,random,re
sys.path.insert(0,'/home/claude/p5')
def wrap(text,W):
    lines=[];cur=""
    for word in text.split():
        while True:
            if not cur:
                cur=word;break
            if len(cur)+1+len(word)<=W:
                cur+=" "+word;break
            room=W-len(cur)-1
            if room>=3 and len(word)>=4:
                k=min(room-1,len(word)-2)
                cur+=" "+word[:k]+"-"
                word=word[k:]
            lines.append(cur);cur=""
    if cur: lines.append(cur)
    return lines
def cut(grid,rng):
    H=len(grid);W=len(grid[0])
    occ=[[False]*W for _ in range(H)]
    out=[]
    for r in range(H):
        c=0
        while c<W:
            if occ[r][c]: c+=1;continue
            e=c
            while e<W and not occ[r][e]: e+=1
            run=e-c
            opts=[w for w in range(3,8) if w<=run and (run-w==0 or run-w>=3)]
            if not opts: opts=[run]
            w=rng.choice(opts)
            h=min(rng.randint(1,3),H-r)
            for i in range(h):
                for j in range(w): occ[r+i][c+j]=True
            out.append((r,c,h,w))
            c+=w
    return out
def main():
    seed=int(sys.argv[1]);P=int(sys.argv[2]);pref=sys.argv[3]
    rng=random.Random(seed)
    from clean import paras
    docs=[p for p in paras(sys.argv[4] if len(sys.argv)>4 else '/home/claude/corpus/wt_valid.txt') if len(p)>950 and all(32<=ord(ch)<127 for ch in p) and '_' not in p and '|' not in p]
    rng.shuffle(docs)
    pieces=[]  # (rows, page,r,c)
    dims=[]
    for pg in range(P+1):
        W=rng.randint(40,60);H=rng.randint(10,14)
        text=docs[pg]
        # start at a sentence start sometimes
        lines=wrap(text,W)
        short=False
        if rng.random()<0.25:
            # document ends: cut text at a sentence end so that it has H lines
            for cutlen in range(len(text),200,-1):
                if text[cutlen-1]=='.':
                    l2=wrap(text[:cutlen],W)
                    if 10<=len(l2)<=14: lines=l2;H=len(l2);short=True;break
        lines=lines[:H]
        H=len(lines)
        grid=[l.ljust(W) for l in lines]
        rects=cut(grid,rng)
        ps=[]
        for (r,c,h,w) in rects:
            rows=[grid[r+i][c:c+w] for i in range(h)]
            if all(ch==' ' for row in rows for ch in row): continue
            ps.append((rows,pg,r,c))
        if pg==P:
            rng.shuffle(ps)
            k=rng.randint(3,max(4,len(ps)//10))
            ps=[(rows,-1,0,0) for (rows,_,_,_) in ps[:k]]
        else:
            dims.append((H,W))
        pieces+=ps
    rng.shuffle(pieces)
    with open(pref+'.in','w') as f:
        f.write(str(len(pieces))+'\n')
        for rows,_,_,_ in pieces:
            f.write('|'.join(row.replace(' ','_') for row in rows)+'\n')
    with open(pref+'.truth','w') as f:
        f.write(' '.join('%dx%d'%d for d in dims)+'\n')
        for _,pg,r,c in pieces:
            f.write('%d %d %d\n'%(pg,r,c))
main()
