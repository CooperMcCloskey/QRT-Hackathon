#include <bits/stdc++.h>

using namespace std;

string ltrim(const string &);
string rtrim(const string &);

static const char*WL="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";
typedef unsigned long long u64;
namespace DL {
const int K = 27, MAXH = 14, GW = 64, NWD = 64, MAXPL = 320;
float lp1[K], lp2[K][K], lp3[K][K][K];
int minW = 40, maxWg = 60, minH = 10, maxH = 14, minPW = 3;
double TLIM = 1.6;
float LINK_T = 6.0f, LINK_D = 3.0f, WJ = 1.0f;
struct Piece { int h, w; vector<string> rows; vector<int> tr; vector<char> allsp; bool lead, up0; int cid; };
vector<Piece> pc; int N;
chrono::steady_clock::time_point T0;
double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }
inline int dec64(char ch) { if (ch >= 'A' && ch <= 'Z') return ch - 'A'; if (ch >= 'a' && ch <= 'z') return ch - 'a' + 26; if (ch >= '0' && ch <= '9') return ch - '0' + 52; return ch == '+' ? 62 : 63; }
inline int cls(char ch) { if (ch >= 'a' && ch <= 'z') return ch - 'a'; if (ch >= 'A' && ch <= 'Z') return ch - 'A'; if (ch == ' ') return 26; return 27; }
inline bool isUp(char c) { return c >= 'A' && c <= 'Z'; }
inline bool isLo(char c) { return c >= 'a' && c <= 'z'; }
inline bool isAl(char c) { return isUp(c) || isLo(c); }
inline bool isDg(char c) { return c >= '0' && c <= '9'; }
inline u64 mix(u64 x) { x += 0x9E3779B97F4A7C15ULL; x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL; x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL; return x ^ (x >> 31); }
struct HS { vector<u64> t; u64 mask;
    void init(int lg) { t.assign(1ULL << lg, 0); mask = (1ULL << lg) - 1; }
    inline void add(u64 h, int b) { h = (mix(h) & ~7ULL) | 4; u64 i = (h >> 3) & mask; while (t[i] && (t[i] & ~3ULL) != h) i = (i + 1) & mask; if (!t[i] || (int)(t[i] & 3) > b) t[i] = h | b; }
    inline int has(u64 h) const { h = (mix(h) & ~7ULL) | 4; u64 i = (h >> 3) & mask; while (t[i]) { if ((t[i] & ~3ULL) == h) return (int)(t[i] & 3) + 1; i = (i + 1) & mask; } return 0; } } HT;
const u64 SW = 0x1111, SP = 0x2222, SS = 0x3333, SI = 0x4444, FP = 1099511628211ULL;
inline u64 hstr(const char* s, int n, u64 salt) { u64 h = salt; for (int i = 0; i < n; i++) { h ^= (unsigned char)s[i]; h *= FP; } return h; }
static const char*TG_DATA=
"ybvJrOq9rMqumtzs7Oes682+3qnn/JjI3M6e2O3179qf7vmf3Pn++IzPjv+or+p/79q5/9+v+97n7enf7fvO69v+/aK+yd3O78vZ7Jjr3N1e+v+p//i//vef"
"rvv//pT+jd+p/+ur/8mt799P6O/z//jv3vnPzvz+/bidq7zv37qV7Yro+/3N9v/3//r//vXf7/b//6z+Xt6N78zK/p3e789f2K7X7viN7dnvu9v++Xfabv6f"
"/76e+c6+/89/6orZ6Ovdzuvvmtz+7dXN3Nje3qrH+7iavN6O6O7nvvmf/aiPrPr//ervz/7O///+7/8P///P2bzW7PnMzNnPqtz++8fOn/6a392u7K64798/"
"6v3oX/nf7vm/y+z97Kerq62+7qno+4f4/u7O+e/y//b/7+rv/v7//Wzvfv547+2a/szu7++f6fnZz/jv/3z+f+zu/cbere7P/8y9/L3t7/8e6P70vujO7tnf"
"7+zuq4iqy8qq3buc65vXrd/+y7uP/f7Lvf+/XN///6Xbrf+e//Re/7y/78/P/Onnf/rW/+29bu3//L3eeu2d/b2+38zO2s9P7K/v/f9PnPePytz/39jvr+jf"
"//7f/+ni/++/9t/z3Prfvfqv/fn//2z/jv+tz7qb//zN3/9P7p3P+/2PO/+Puu/+7pv/n//P//8f///P//+/+t/i7/fe3/zv7Pv+/8jrre2f39a//Z3a3d9P"
"uf7z//nf7Jr/+/3//civw76+z/7M/7/a779u//nvq/ybzu+fmezOnZXvj+6p/+2u9LnZ/9+v///+n/j////5//P//6W9ib6uv7rM/aro7a+P/f35z/v87tv/"
"mf3//tLPb/5q/+3P/t25/+9f3q3e5/6erf+fhN/v/5v/L/9/7/+u//3+/6/f4//d/vvMnf7Pyv///Kbvn/9P//yf/v/9z79f7N/n/vrO3fzv6P7//4L/n/8+"
"//+v///d/5+pqqmtu9utupuumM367e+K7N7d3sj4/Xjn/7+/+dj1//eP//nf+v7/+Lz/j//t7/2v/97u//8f/O/7//b///jv+vb//rWfm6qs//rs76b5/t9/"
"9P/c//jf//bP3/3//8b/Pu/M//3d/9je/+9P8f/7//3///vP7/j//6m8u6//7+ao3qjn/f/O/f/w//3///3///3//7z+ft49783L/q3e799/+f/z//X///r/"
"//r/+U7+n89P/+/b/u/vr/+v/t79/fn/7/b/3vz/+5Ls6e/O78ua/shovLzf+//73/c//tu/3f3//ufvz/7O///+7/8P///P9//4//T///b///n//d7OT/6f"
"7/+f/O/Y//9f9f/J3/6v//qf+f//+/be7O2P/4vc/5jz/9/P/P/4//H///3//////q7/X//N//7e/57///8/6fnZz/jv/3z+f+zu/fb/7/7v//7+/u397/8O"
"6P70vujO7tnf7+zuq4irysuqzbur64vZnO7+3s7u/f1tea+Pitr//tv+f//f//3f/x3f/9+v+P/07/u//vXv7/f//+3/v//f//9P/57q/+8//J7t7/u+i76f"
"6P///hT/z/3P//2///zd//+P/f/63vev7/2Pvvb4/mbujf+f/96s//u+7+9P5p63/v+vnKy/er7/z27/f/+v/+9f//1u///P3d/o3Pq/zu3f6P79/GL/f/+f"
"//+P//9P/9/v6P/7//r//u7//vz//6Hrrd9P//6Z/5/t/79/7O7u7f2fVc2Pzbju/qz/z//W//2f/Ujd//9////+//7///////D//47/T/9f//9///+v/9/v"
"/f/9//3/////3P///8D/f/9P/+yP/6uv//9/zP/s//1f2p1viO7//63/bv+f///O//7v/98v+//5z/vv3/zvnv///6GupP2d//7P+P/I7t6PzvX////Pzd6/"
"vP///4LvP/+9/+7P/v7u799dqLmru9q9yqqumb367v/dzd/N34v4/NnI2m+O5v/3//qP//qP/vn/+ivv7//s7/6P/f7O//9//P/1z/SP//yv7P///Lqs2Nvf"
"/8vJ6ofs+/5+9P3r//jv//XP7/n//8v/D/++78z//93/3+/v8//3//n///jP/83//5itmsr/7921/XrJ+u/e9v/0//v///f///n//wn/r//e/+7r/97v/+/P"
"+f3z//jf//z/////9X7/j/8v//+v/v/P///f9u70/vrP//v/7v3//+Wtrc7O34q3/Mqsfd2N+f/67/hP/9fP7v7///f/7//v//////8P///v9f71//j///j/"
"/vn/7Ove7v/s7+6//v/s7/8P/v/+j/7//Po//q7//5ZMne7v38rc/aX8/v/P85/1//j///r//////yz/r/9e//++//z//8/f6vva3/nv/4z/n+3u/fLf7++/"
"7+f5/87+7/8f6f74z+bf7uvv7+3+zIO5y8qa3bys66vXre/+75ru7P+rqt9vh73+/4r8f//P/+2//6R8/8+/+f35r/jM//a/X/r//t7/zt+/7+7e/93f7+8P"
"753v7v25S7+fm+/978f/X/if/vtv/92p/++f9f/p7PXfzfqv/Pn/+mv9j/8///19//vf/+/P7t3v+O/Obf9PnZ//36z/L//P/72P//+v//+//v/67/qd3vz/"
"9////pLuetue78ev/K7b/Z+PeP/3//nv3Yn/+v7//Mieqa7O7u7M/q3U7u9e773P7P+PnV9/28r//4f+n/97/+mf/dmm7++v//////////////D//6/djtyu"
"783K/Yzr3M9O7uv53/vv7r3+ePz+/6Pdj/6Z/+3N/qqpn79e37z+/P7buM9Pic/P/6v/H/9v//+v//7v/+/f6v3ZvvrP7d3f6P///5Kfr/6c///f5f+2/++f"
"3d7q//7v7evv7P/+/8H9j/+u//+///y/++8dqbmru9q9uqmumdz67e9t3f+f74n4/6nJ6u6v1+747+O//vq/7fr/+s1fz//c3/2//+3c/+8v9N7p7erv7vzO"
"7Oz+/YSbmu/f/7r5/6bsrt+e/P/0//XP/Prf+/7//ob/vu/D//3d/9zO/+9f6O/z//jv3vnPzvz+/dh+naz//vjn/8X8+v3v8//3//z//vXv7/n//73+ft6d"
"783L/q3e798/+P/2//j///X/7/j/+R3+z//f/+/f/e/v/++P8M3t/f3u7/3/3f7//vvf/+////m+/+GP3/3v8f/73/nP/tuf3f3//uzvz/7O///+7/8P///P"
"+f/4//n///L///z//+7vr//Y///f+u/u//8f/v/yv/zv//7//P/9/fXfzs///sT3/5f6///f+e/y//b/7+rv/v7//Rz/r/+r//68/93+///P6/vX3/r//53/"
"n+3u/uP/7/9f//7v/+////8v6P70vujO7tnf7+zuq5irzMu63ay866zUvd7+3639/Pifdt6fnL3+vav/z//v//2v/+3f/x9/+v36f/zM//nPs/3//pz/z//t"
"//8f/P/v///f7Z/u7/6/jOuPqP7+/9T/z/0v//2P//iN//+v/P/y7/mP//2f+f///Mr8v//v//uv/+7Tv/9P2unb/v+/Svi/uo3/74z/j/9P/+9///5+///P"
"/P/57vjezfLv/P7+/Vn/T/9///+f///f/5//6f/x//7//ur//vz//4n/X/9v/8+v/7/e//9f6Y/M/fqfnemfrH39/pifv/+d/+xv/dtN/+/P/v7s7/z/7//+"
"//D//0z/X/+///9v///f/9///v7/z/rv/u7/v/////D/7/+y//9P/f////3M6N3m//efjO6f2P///E3/Pv9///++/+7v/9/f+f/z3/v/7/1P/v//+6uur/09"
"//7P+P/I7t6P/P/9//3f3H/v/f///3H/j//C//7P/v7u/9/NuLqsrNnNy7nOatz67f/tyO7P36vo/JnV2e/e+f/6//bv//LP/vz/+57vv//X7/5///Tt//9/"
"+P/7//z///U///7/z8rv7P6//8zr/8n+788f+v/d//rv//iv3/P//3X/z+/a//7e/+3e/+8f/P/zj/zv7/bv7/7+/tmNza7v76nY+Uv7/f/e9v/3//r//vXf"
"7/b//4z/v+8+/+7t/97v/+9P+f/0/vjv//v/////9YjPT/+f//+f/n+f/f+f/Oz8/vf/z/n/+Pz//+Pd3O7f75mr+5hrju+P8//8v/2P/9qv3v3///b/7//v"
"//////8P//+f+//0//fv/vT///z//t6v3//d///f/9/6//8f/v7Zz/q/3v3v6f79/rHc3L7f73zo/oT7/f+/9P/5//P///3//////i7/T//d//+////////v"
"6/va1/rv/53/n+3u/sP9+b///5z/+X/67/8/6f74z+rf7uvv7+3+zHKbu8uq3byc65vXrd//3u7/7P5fWu7Pnf//76brX/+O/+bN/9qP3+/f9v75f/rK//zv"
"q/v//ab/O66//+3d/87O7+9f74y//f+vj/+fpK3576r/f/df//y9/u7J/7+P+f/8PPj/bv3P/vv//hv/r//P//uf/87L///P+v/f//bd3///7v///0H/z/9P"
"/6+///6f///v/P/x3/rs/vzv/v///qv+ee6d35XP/87b3a+OyP/0/+n/+4r/+vz//7ivq1y+3u7d/7/a/e9P7s7//f/fH//P3fr+74vtr//N/7nf+Y7Z799P"
"//////////////D//7++at6/38zO/G3r7u9f7OzqnvrO7Nvfiv3//qPfn/+G/+zO/57L/599/9z////v4v/f5////5n/H/9v///P/////+/f8//0v/vv3/jf"
"/v////z/f/+///+f/v/3//8v8u79//7v7urv6vb//2n/L/+t/++v///f//+9uKqsrMm8uruumd357v99/c+u3331+Mv/zL7L2O3179qf7vmf3Pn++IzPjv+o"
"r+p/79q5/9+v+97n7enf7fvO69v+/bI/2vr/7t3q/qjci//P+/+6//nF/vivvvz//pX+nt+p/+y8/8q1799v5O/0//nv3vrPzv3+/anq3cn8/3jk/+/M//+v"
"9v/3//r//vXf7/b//7z+ft6d783L/q3e798/2b7U7vme/ervy9z++kjsb/+//96+/N/f/99f6orZ6Ovdzuvvmtz+7cXqzv+Fz/36/6Wf7q/f6e7ozvqf/bmP"
"vPv//uXvz/7O///+7/8P///P7d7r/vzu7uzv3e7//cDOn/6r792+7L7E799f/P77n/vv/vvf3f3x/br9+e/v3qb1+2n97f7v+e/0//f/7/vv//7//WTvfv54"
"7+2a/szu7++f6fnZz/jv/3z+f+zu/cbere7P/8y9/L3t7/8e6P70vujO7tnf7+zuq5iK3Mu426vZ+an8vf//vr/ty9rKa56PnNz83Mjuv//f/v1f/+kv/8/v"
"8+z5n/u7/vi+rfz//av/zu/f//4v/+bf/++v7H/r//2vnc2fmd79+mX/3/3P//1P//xd//+v/v/97/7v7/4v/v3//1T/f//P/79//9+f//9d/svr//2OLN7P"
"u/7/7zr/j/+//+9v//5+///P+v/17vjv3v7v/f7//pP/Xv+P/96v///f/z/v+//9//7//+z//////7D/jv+v//8f/+/f///fzq2tz/zMicyPnHv830j/z/99"
"+eh//ezd///f/v7s7/z/7//+//D//yz/nv9v//+//++u/6/f7v/9v/z//fn/z////1H/z/+X//5P/93u7+/Prc7d/f2NmK9vuv/8z1j/fv9////P///v/0/f"
"9P/33/j/7/nv/v//+5een/2M//7P5//H7t5v+v/4//+d//qP/f/9/4PuT++L7uye/f3O7r6KqLqsvNnNy7i+idz57f+b7M6u387m7ajHzY6++//5//3P//jf"
"/vL7/Xr/3//I7/5P/9xu/9+v7f7pv/nf3u2v+/79/4Gtybze777b/ora281O+//9/uv//8yvz/z9/2H/j/9O//ye//mf/8/P+P/y//j///nv7/7//5ycnKr+"
"r76n7Y/Y+v7M+P/5//b//vfv7/T//33/j/+L//v7/5/s/69P2v73//j/7vn/6/3+98T/j/6v/96P/4/P/88/+97z/f7/7/7/2v7//MSd/Z7e/8+n+pq7e7/P"
"/P/Xb/rf//jfi////+Xvz/7O///+7/8P///P/O/z//z///X/7v//997uvu/c7/9P/+/r//8/+u/5b/ju//vf+/n/6sW5hN3e3p3b/pz5/v3O+P/x//n///3/"
"/////w//7/+u///O///////P6vra3/nv/4z/j+xO/ejv/u/f/87t/s3u3/8O+P723/vv/uzv7/7+3YKau8qr3buc6prZrd/+7qz+2rmbXd+Pmv7f6pz/L/+f"
"//mv/8vO/++/+qS52f2bqszf7f///7v/rv+///9//77s/+wv65397f7Pa+6fqf/e/ZX/3/2v//4v//p9//+/if/83vrf2v3f7v3//hL/n//P//+e//7u/++f"
"/Knc+/9tXv/PmP2/7lv/r/9f//+f//6f//9/+v/7Lvyv3v7v/f7+/pb5bv/f3/6//6/O/49P9//1//j///jv/vj/+1rvju+L//3f/4/+//9f3q3+7f3NbOxv"
"mJn+3339n++a//aP/7q5/++//v/t7/X////+//L//5zv3v/v/++5/27O/+8v3v7o3/7+/+3/v////mH7vP/b+/+/z+zO8++v/sj83f6Pev+vxP/+7s3/n/+P"
"///f8v///++P+P/3z/nv3/fv/v///6Sur/2d//7E+P/I7t6P/vr8///O//+/qf///4HuT++L7uye/f3O7r6KqLqsrNnNy7q+ad3p7u/c667O34X6/crW2++d"
"9ff3//7P//nP//b//t//P/+Z//iv//rb/8/P/P/p7/rf/vzf6/z//rHOx93f79zc/onbnMxe9//3//hv//av//v//9z/rf7O//rf/63M7/8e9Pz4+fuf//r/"
"/fz//6afrrv/772mzY7X+vzL+P/4//r///X///T//73/b/2d/+u6/43/7+9P+u7a//n///v///7/8az/Df/P///P//3v///f6P70//b///jv+/r/+urd7d7P"
"76za/celiu6O+f/97/h//8Zv7vj9//z/7//v//////8P///v+v31//n///nf7v3/89ven/yL793P+/+33t8/+f/nvvjP/vqf+fv+/IWuffzP7l3c/ov5/v7e"
"+f/z//b///b//////k/9j/+G//+u//r////f/f2q7/T//87/z/7//sPu7//P/67P/8/s3/8f9f/27/j/3/n/////6Yiru8ur3Lyc65vWrd7+/1jv3v+Mvt+O"
"ie/f/43bX/+f+vm//3zb/d+f9vj5z/jW//3Pj/r//azubN6d/+2+/q1+359vz7/v3/3Opu/fdN3//fn/3/rv///v/+/8//8P+e/o3fffn/1f7Pv/+Jr/r/+f"
"3+vB/9////+v/pn//v+fL/+vuO/v/93/D/+f///f///v////7P723/ne//vf+f3//aPuiP2P3+eO/s+t+8+fuv/n//r/6Hzv7P7+/aTPm53P7uzd747q7d8+"
"727P/v+Gmq+fjN///8rvX/+b77mv+a2b/9+P/v/+//7///////D//729mc6ur5zc/b3o379f6/323/jO7bz+V+3+/prsf/+l//3P/77q799f393/+P2fb7+P"
"aO/v/7v/H/9////e/+/////v+9727/mvXv/v2v///qb9r/lP///f/v/9761P9ry1/vvP2/3/2f3/31avb/+s/P+P+/+//++Zmaq7rNq9yquued367f+d/c+e"
"79ro/JTJ3c/P7f/y//3f//uv+vb//Zvr37+t7/pP/6vt//9fse/7//3///uPzv7//ouPye7//+yo/sTq7++e8//a//uf//zf3/r//9X/3+/d//7u/+Lv/+9f"
"+O74//bf7+i//P3/+Lh/m9//3/qY/YnH/v7e+P/5//v//vfv7/j//8P/f+8u793c/87v79+f8//1//n///n///z/+57/H//v//zP/v////+v/crk/Pru7/j/"
"3e7//vTvvu+e3+m5+WXLrv7e+/70/vqf/vWP/////O3vz/7O///+7/8P///P+v/1//f//+P///7//u7/n/+s//+v/f/c778f+f7lz/bf//vv+un//OfV3e7/"
"/sb6+6j4///c+eXz//f/7/vv/v7//S3/v/+M//6d/6r////P6fnZz/jv/3z+f+zu/ab/7/+//9/s/+j///wf6P70vujO7tnf7+zuq4i6y8qa3byM65vXne/+"
"31zu7e2+rd8/u93+7YvdXv+u/eme/8md74/P+ez5j/q7/vi+nfz//bXvfd6e/t2+77y+7d8v/Q7u/v7v3u/P7e/u/sv/v/yv//yf/+vM/+8f6d/4nfq/vvqf"
"3Pr+/ST/bv+v/+6+/+3d7++v6r3t+//fXu3f3ev/v2P/f/+v/+9f//1u///P6u/l3fjOrOzf6f3+/IXteu2O39ie/b677Z9/p+3m//nv65nv7Pv//Le+qJ2+"
"3u21/77Z7t9u3cyN7e2teryPq8nr7Yjufu6b/9mP+siq/9+v8v/t7/3////+//T//53Na82ez8yc/azK7b9/7Pzpr/nd7cruivv+/aPej/6V/+2e/7vM3s9+"
"9f/k//X///zv/////57/Lv9v//+u/u7v/9/P9u7nj/ferunP7P7+/pmen/2M//7P5//H7t5v7N3q/vzP3Mvf2/7+74HuT++L7uye/f3O7r6KyLudzunt1eu/"
"m976/9+M2sud3bjn67znzM+t9//3//bP//jv+/r/+cr/X/+j7/3P/+qt/9/P++737/jf3vzv5/7+/pOdqcze7rrq243r299e+P7J//rP//Pf7fn//4v/P/+d"
"/+y+/+uv/7+v9f/5//n///b/r/3/9ZyKi63+38uo/J/Y+v7N+v/5//3///fv//X//8T/b96e/+3t/o/u7+8/2m/q7/f///v/7P7/9Fr9b/6e//2v/5/P/69v"
"6O7n7/jv+ev/6v3+/6S76s3e3Gq66q2Kqs3O+f/4rfu//fSP2fz///v/7//v//////8P///v+P/13/T/7/j///3/+c3un/6b7/6v/e/X/u8v6f/of/jP3PzP"
"+fv+66VsvK2e7an4+3z77+/d+P/0//T/z/v//////i7/f/+J//+f///////v6/vV3/Tv/53/n+3u/sne/e/P/+zf/d/87/8P+f/zj/3v//j///7/6Ymqu8qq"
"3buc65vXrd/+zru/6/idiL+uq7rb+lrfn/1///6a//yP/5+f+f/5f/nf//Zv/vn+/13/n/+P//+P++7P//9f2Xrr7v6vnL+cuq3e/bb/b/+P//6f//4//7+/"
"/f/93vvv7/Z/uvT//pruXv9v7859//zO7u9vu5rd6f/Petffir6/z53/n/9v/+8///6O///f+v/1z/b///2/2vv/+Df/j/+P//+P///f/4/f8//6//j/7/n/"
"6+3//3bvfu+f//pv/+/e/89f7qrf/v2fWNy/3tjv/4f/T/+M//p+/urP/9/f//////////////D//zrfvv+f//+v/++O/+9f+v+37/Xf7env7/r9/XX+j/+e"
"/92e/7i/799Pmte6+vyeqo5/6+//733/f/8///+6/////++f9//07/Xf/+nf//3//66un/2d//7P+F/I7t6P3svv//+fmN+v5P///8avj//N//7P/v7k/99N"
"p7qsvNnNy7m+it35/v+q/b2er9j4/cjX7c2v8f/9//3f//mf/Pz//cz/7//w//2//////9+u+e/8/v3/v/Tv+f7//rTOxt7f/6vp/aXu7f2P+v/s/Pif//Pf"
"7/j//1z/j+/d//5O/6mu/++f+e/x//vv/vvO/v3+/rievtz//5tH7b37+f/e+v/0//z///Tv//r//6n/X+8+/+6t/97v7++f+f7z//v///7/////9F3/L/+v"
"//6P/v/v///f9O/z/vr///3/7vr///zc7d/v78zK/OjNvu4v+f/77/2v/+Sv7vf//+b/3/7e///17/8v///P9P/4//f///f///n/+u7uz//O79/f/v/d7v8P"
"9//03vl/7/q//P///Jrd2M7v/83r/rL6///e/v/8//zf//7//////6DfT/+u//8v//7//+//6vva3/nv/4z/n+3u/eLf///u/+ru+f3///8f/P/17N38/Ln/"
"//r87YOry8ua3byc66vXnd/+36//6v1Mrv9fnv7//s3rvd2++rPe/n3L7u+P+/j3X/u67/7vX////77uS79P/vzO/8/v7q+v7o7b3v6vfv/P1+7+znP/7/K/"
"/cy///7N///Py//8Kf7P/v3//fj//zz/r/jf//9///7u//+P/qr9/v9fn7+faM//353/z/8f///P//+f//+f+v/0//r/7fyv/tv//GPepty+39ne/d/X/q+f"
"af/o//z/yqv/+u3//8Sepa1+zu3b/b3X3v+v///v/8vvzs+Pa/X+2MnunN+t//uu9qzr799P/v/t7/3////+//D//62sbc2O3+65/Kra/K+P2+z1v/i+797+"
"e/z/7sXOju2X//3O/r3M798+3Nzs/P2+pc6vuv//74T/L/+P//zv//n/////8//5z/vv3/nf/v///+Xvn/+u///v+/+q//8fnP/6+Prvz/vP+f///6P93/+a"
"/7+///+v/+9TuLq9vLq7y7uul93p7f++3M+v3uX3/rfX//+v2O3179qf7vmf3Pn++Jzfn/+5z+yf/+zK/+8//u/9//3///7v+/7//tDv6e/t/7n4/6T+/99f"
"+/+6//nP//mvvvw//kb+nt+p/+y8/8q9799v+/94//vv7vzf0/3+/reeiN/f7vmn/nvq+//e9v/3//r//vXf7/b//zz+jt6d793b/r3e799/5M/q//W+/uj/"
"3e3//Hrabv6f/76e+c6+/89//d79/frv7/v/3f7/+vGP/t9/z+Xr/9ip36+//P/73/zP/tu/2f3//uHvz/7O///+7/8P///P/e70/v3u7vnv3eP//uzvz//d"
"X+7f/t/c/+8v/f77n/zv//Hf3f3+/ejuju/vz+H9/t37///v+e/0//f/7/vv9P7//W3vfv547+2a/szu7++f6fnZz/jv/3z+f+zu/bb/7//f///9////v/8P"
"6P70vujO7tnf7+zuq5iLyLuszbqq65raq//+7+///f2uvu+Pot/+6Y3un/+f/vhf/9nO/8+P8/793/3u//nf3v72/+b/T/+////P/47q//8/2q/4/vyfvu8/"
"2a///qv/3/7f//5P//1+//9P9u/6vfxv3vxv7fz+/aj/X/8///9v/////9///7zN/f6efe/vK+//727/f/+v/+9f//1u///P/f/07vrv3v7v9f74/qf/P/+v"
"//zf/7///29/4v/2//f//t3/+/7//vz/nt7f//ze/579398f/+/////fqvpP/+f//6X/z//d/+wf/ezd///f/v7s7/z/7//+//D//8z/n/8v//9f///f/9/v"
"79//7/797pz/z/z+/+H/3//T//9P/+7r/++v7d3t/f3fet/PnP//b5P/Lv9v//+u/u7v/9/P+/8lz/nv3/rv/v///52en/2M//7P5//H7t5v+Pn1//7ve+b/"
"/v///4nuT++L7uye/f3O7r6KubqrrMrNyrq+idz5yv+f/83//z35/278/f/P7P76/+3P//HP7fn//I3/P/+Y//fP//3O////+97p7erv7vzO7Oz+/fFv9fz/"
"/73H/4n8//+v+v/t//3v//Hf7/7P/5r+nt+p/+y8/8q9798/9P/8//L///7v7/v//819yv6//2vZ/276/v++9v/3//r//vXf7/b//6z+Xt6N78zK/p3e789f"
"6s7i/vq+/evv3I3++3rabv6f/76e+c6+/89/65va6evd3vvvq+3+7fL/+v7//+6R+u3r7v/v+f/z//1v//mP//3//+//3/7e////7/8P///f2bzW7PnMzNnP"
"qtz++8fOn/6r792+7L7E799f3P/1n/v///1/+/r//AXu7u/v/+79/sr+//+/+//3//L///z//////pb/Pv+q//6M/934/++/7Pzr3/v//63/r31//uTv3z/v"
"/67q/u7+//8/6P70vujO7tnf7+zuq4ibu8qr3qub+5vZnM/9z+3/2/9dfO5fvO7/76n/b/+f//pP/+bf/6/v+f/3j/49//jf7////53/b//f//un//Xv//9/"
"42/9//+/7/1vvP7+/4v/r/7f//7P//U+/7+/+P/2f/rv7/lv/vr//Fv/bv+v/+5+/+1t7++v/9/7////D///7P///2v/f/+v/+9f//1u///P/f/27pvfzfXv"
"/Pn+/lbvX/+P//uP///f2f/Pi/32//3/u1bv/f7//1qveu/P/9+b/9/4/91////P7v/tjO9vzePv/6v/T/95/+6t/83W///v/v7s7/z/7//+//D//4z/jP8v"
"//+t/8yu///f/f7s7/ju/+v/W//+/9P/n/+i//+P/+v9/f/fycv09/zKvu+f6///v6r/Lv+P//9u//7v/9/f9//5b/z/7/O//v///76+r/0t//7f+f/Z7u6f"
"7O3r/v3f3dzf3PP+/2P/P//N//7f//7v/+9tqLqsrNnNy7m+id367f+Z376+z5vZ+9n07/+f5/72/+e//vW/7fv/+q3fr//Kz/wv/+zb/+/I+97n7enf7fvO"
"69v+/ZLd1M7f7uz5/qnt7/9u+v+p//i//vefrvv//qT+Pt+6/+y8/8u9799/+f/5//v///Lv7/n//9zqv9///6nC/839//+N8//5//z//vXv7/j//73+jt5d"
"793b/r3e719/7e/h//rf/v3/7v7/+y3sn/+//95+/N7f/9+v/Ezr6uXu3vzvrO3//fj7z73P/5yD/dnur/+v8v/63/xf/su/3f3//uzvz/7O///+7/8P///P"
"5t3q/vzu3ezf3e3//cLOn/6a392u7K64798//P73n/vv//vf3f3+/eLeju/vn4r4/tf9//9r++/2//n/7/Lv//7//X3/Lv+q//68/93+/++/6vra3/nv/4z/"
"j+zu/eP/76/P/27v/u////8f+f/97/hP//n/////63Way9ub7Lu9+4vZnN/+zsu+7P2fXM+fmdz974fuX/++/+uf//mP/3/f5+/6j/uv/uSf//v//Y3vTv9u"
"/f+O/+qf7s/P+LzO/fufer/M2pt//Zz/n/9///pP//et///f5v/33/m//udv//n9/l3/X/9v/++O/v+v/9/f7t7f/v3eK/7PiO///1z/n/+v/+9f//1e///P"
"+O/4vPXfaeq//+z9/Vv9b/5v//1u/+/O/9/f5O347vju/dbf7+n//HvdXO6t/+5P/u6e79/P3twv7/7Obr+fvrvt/33vj/68/+h//uWu/+/v/f7a//v/////"
"//D//4zvLf6d//+O7v+v/+/f6vr4n/nd7ajtfvn9/Mj/v//C//9///vfzt/f/+7u7v7MPX/Pxv///nv/b/4+//6P/+/f/+/P9f/4f/b/7/m///7+/s7qP/1/"
"//2v/v++2/1/+N/z//3f/uT//uv//439X/+Y//9t//zP396t3e3e3u3u7u3vze79/w8="
;
void initLM() {
    vector<string> words; vector<int> bk; int nb = 0;
    { // decode 5-bit symbol stream
        int L = strlen(WL); vector<int> sy; int acc = 0, nbit = 0;
        for (int i = 0; i < L; i++) { acc |= dec64(WL[i]) << nbit; nbit += 6; while (nbit >= 5) { sy.push_back(acc & 31); acc >>= 5; nbit -= 5; } }
        string prev, cur; int pp = 0; bool open = false; size_t i = 0;
        auto flush = [&]() { if (open && !cur.empty()) { words.push_back(cur); bk.push_back(nb); prev = cur; } open = false; };
        while (i < sy.size()) {
            int s = sy[i++];
            if (s < 26) { if (open) cur += char('a' + s); continue; }
            flush();
            int p;
            if (s == 31) { if (i >= sy.size()) break; int v = sy[i++]; if (v == 31) { nb++; prev.clear(); pp = 0; continue; } p = v; }
            else p = pp + (s == 26 ? 0 : s == 27 ? 1 : s == 28 ? -1 : s == 29 ? 2 : -2);
            if (p < 0 || p > (int)prev.size()) break;
            cur = prev.substr(0, p); pp = p; open = true;
        }
        flush();
    }
    int lg = 16; while ((1ULL << lg) < words.size() * 130ULL) lg++;
    HT.init(lg);
    auto isV = [](char c) { return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u'; };
    for (size_t wi = 0; wi < words.size(); wi++) {
        const string& w = words[wi]; int n = w.size(); 
        int bq = min(bk[wi], 3); HT.add(hstr(w.data(), n, SW), bq);
        string forms[4]; int nf = 1; forms[0] = w;
        if (n >= 3) {
            char l = w[n - 1], l2 = w[n - 2];
            bool cy = (l == 'y' && !isV(l2));
            if (l == 's' || l == 'x' || l == 'z' || (l == 'h' && (l2 == 'c' || l2 == 's'))) forms[nf++] = w + "es"; else if (cy) forms[nf++] = w.substr(0, n - 1) + "ies"; else forms[nf++] = w + "s";
            if (l == 'e') forms[nf++] = w + "d"; else if (cy) forms[nf++] = w.substr(0, n - 1) + "ied"; else forms[nf++] = w + "ed";
            if (l == 'e' && l2 != 'e') forms[nf++] = w.substr(0, n - 1) + "ing"; else forms[nf++] = w + "ing";
        }
        for (int f = 0; f < nf; f++) {
            const string& s = forms[f]; int m = s.size(); 
            { u64 h = SP; for (int i = 0; i < m; i++) { h ^= (unsigned char)s[i]; h *= FP; HT.add(h, bq); } }
            for (int i = 1; i < m; i++) HT.add(hstr(s.data() + i, m - i, SS), bq);
            for (int i = 0; i < m; i++) { u64 h = SI; for (int j = i; j < m && j - i < 8; j++) { h ^= (unsigned char)s[j]; h *= FP; if (j - i >= 2) HT.add(h, bq); } }

        }
    }

    static const float Q[16]={0,.25,.5,.75,1,1.25,1.5,1.75,2,2.5,3,3.5,4,5,6.5,9};
    unsigned acc=0;int bits=0;vector<float> vals;
    for(const char* p=TG_DATA;*p && *p!='=';p++) {
        acc=(acc<<6)|dec64(*p);bits+=6;
        if(bits>=8) {bits-=8;int b=(acc>>bits)&255;vals.push_back(-Q[b&15]);vals.push_back(-Q[b>>4]);}
    }
    int i=0;
    for(int a=0;a<27;a++)lp1[a]=vals[i++];
    for(int a=0;a<27;a++)for(int b=0;b<27;b++)lp2[a][b]=vals[i++];
    for(int a=0;a<27;a++)for(int b=0;b<27;b++)for(int c=0;c<27;c++)lp3[a][b][c]=vals[i++];

}
inline bool ends(const char* w, int n, const char* sf, int m) { return n - m >= 3 && memcmp(w + n - m, sf, m) == 0; }
inline int wHas(const char* w, int n) { return HT.has(hstr(w, n, SW)); }
int inDict(char* w, int n) {
    int r = wHas(w, n); if (r) return r;
    static const char* SUF[10] = {"s", "es", "ed", "d", "ing", "ly", "er", "est", "al", "ness"};
    static const int SL[10] = {1, 2, 2, 1, 3, 2, 2, 3, 2, 4};
    for (int k = 0; k < 10; k++) {
        int m = SL[k]; if (!ends(w, n, SUF[k], m)) continue;
        int sn = n - m; if ((r = wHas(w, sn))) return r;
        bool eg = (k == 2 || k == 4 || k == 6 || k == 7);
        char sv = w[sn];
        if (eg) { w[sn] = 'e'; r = wHas(w, sn + 1); w[sn] = sv; if (r) return r; if (sn >= 4 && w[sn - 1] == w[sn - 2] && (r = wHas(w, sn - 1))) return r; }
        if (w[sn - 1] == 'i' && (k == 1 || k == 2 || k == 6 || k == 7 || k == 5 || k == 9)) { w[sn - 1] = 'y'; r = wHas(w, sn); w[sn - 1] = 'i'; if (r) return r; }
    }
    return 0;
}
// calibration tables: hit [cross][type][len][bucket], miss [cross][type][len]
#ifdef CALIB
float RH[2][4][10][4], RM[2][4][10];
double CN[2][2][4][10][5];
int g_true = 0;
#else
float RH[2][4][10][4]={{{{0.0,0.0,0.0,0.0},{1.3,0.0,0.0,-1.5},{0.8,-1.5,-1.5,0.0},{1.0,-0.1,-1.1,-1.0},{0.9,0.6,0.2,0.2},{0.7,0.5,-0.2,0.7},{0.4,0.9,0.6,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0}},{{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.1,-0.3,0.0,0.0},{0.6,0.2,-0.7,-1.1},{0.7,0.7,0.1,-0.8},{0.7,0.8,0.7,0.6},{-0.7,0.9,0.5,0.5},{0.2,0.5,0.2,0.4},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0}},{{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.8,0.1,0.0,-0.7},{0.7,0.5,0.4,0.0},{0.5,0.4,0.4,0.7},{0.4,0.6,0.4,0.0},{0.2,0.5,0.1,0.3},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0}},{{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0}}},{{{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{1.6,-0.9,-1.5,0.0},{2.6,-0.0,-1.5,-1.2},{2.3,1.9,0.1,0.0},{2.0,2.9,0.8,1.3},{2.1,2.7,2.2,1.0},{2.3,3.0,2.3,1.6},{1.2,3.2,3.1,1.2},{0.0,3.1,2.0,0.0}},{{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{1.0,0.7,-0.5,-0.2},{2.2,1.9,1.1,-0.0},{2.5,3.0,1.9,0.8},{2.5,3.5,2.1,2.3},{2.6,4.0,3.5,2.4},{1.3,3.5,4.3,2.5},{1.0,3.8,4.4,0.0}},{{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{1.6,1.3,0.8,0.2},{1.9,2.1,1.6,1.5},{1.7,2.8,1.9,1.8},{1.6,2.9,2.8,1.6},{1.1,2.6,2.8,3.1},{0.0,3.0,3.3,0.8}},{{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0},{0.9,0.6,1.0,0.0},{1.1,3.9,2.5,1.6},{2.6,4.0,3.2,3.5},{2.4,4.4,4.1,0.8},{1.6,4.5,4.6,2.1}}}};
float RM[2][4][10]={{{0.0,-3.5,-3.5,-3.5,-3.1,-3.5,-2.7,0.0,0.0,0.0},{0.0,0.0,-1.2,-3.4,-3.2,-2.9,-3.2,-2.7,0.0,0.0},{0.0,0.0,0.0,-3.0,-3.3,-2.3,-1.0,-1.5,0.0,0.0},{0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}},{{0.0,0.0,-3.5,-3.5,-3.5,-3.5,-2.9,-2.5,-2.4,-2.5},{0.0,0.0,0.0,-2.5,-3.2,-2.6,-2.4,-2.8,-1.9,-1.4},{0.0,0.0,0.0,0.0,-3.2,-2.4,-2.2,-1.6,-2.0,-1.9},{0.0,0.0,0.0,0.0,0.0,-1.2,-2.2,-1.7,-1.4,-1.5}}};

#endif
// returns bucket+1 (hit) / 0 (miss) / -1 skip ; cap set if capitalised miss
inline int runHit(const char* s, int n, bool lb, bool rb, bool& cap) {
    cap = false;
    if (n > 24 || n <= 0) return -1;
    char b[32]; int nu = 0; for (int i = 0; i < n; i++) { char c = s[i]; if (isUp(c)) { c += 32; nu++; } b[i] = c; } b[n] = 0;
    if (nu > 1 || (nu == 1 && !isUp(s[0]))) return -1;
    int hit;
    if (lb && rb) hit = inDict(b, n);
    else if (lb) { hit = HT.has(hstr(b, n, SP)); if (!hit) hit = inDict(b, n); }
    else if (rb) { hit = HT.has(hstr(b, n, SS)); if (!hit) hit = inDict(b, n); }
    else { if (n < 3) return -1; hit = 1; for (int i = 0; i + 8 <= max(n, 8); i++) { int m = min(8, n - i); int q = HT.has(hstr(b + i, m, SI)); if (!q) { hit = 0; break; } hit = max(hit, q); if (n <= 8) break; } }
    if (!hit && nu == 1) cap = true;
    return hit;
}
inline float runScore(const char* s, int n, bool lb, bool rb, bool cross) {
    bool cap; int h = runHit(s, n, lb, rb, cap); if (h < 0) return 0;
    int ty = (lb && rb) ? 0 : lb ? 1 : rb ? 2 : 3; int l = min(n, 9);
#ifdef CALIB
    if (!cap) CN[g_true][cross][ty][l][h] += 1;
#endif
    if (h) return RH[cross][ty][l][h - 1];
    return cap ? 0.3f * RM[cross][ty][l] : RM[cross][ty][l];
}
inline float junc(char a, char b, char x, char y) {
    int cb = cls(b), cx = cls(x); float g = 0;
    if (cb < 27 && cx < 27) {
        int ca = a ? cls(a) : 27;
        g = WJ * ((ca < 27 ? lp3[ca][cb][cx] : lp2[cb][cx]) - lp1[cx]);
        if (y && !(x == ' ' && y == ' ')) { int cy = cls(y); if (cy < 27) g += WJ * (lp3[cb][cx][cy] - lp2[cx][cy]); }
    }
    if (isUp(x) && isLo(b)) g -= 6;
    if (b == ' ' && a && (a == '.' || a == '!' || a == '?')) { if (isLo(x)) g -= 4; else if (isUp(x)) g += 2; }
    if (b == ' ' && (x == ',' || x == '.' || x == ';' || x == ':' || x == '!' || x == '?')) g -= 8;
    if ((b == ',' || b == ';' || b == ':' || b == '!' || b == '?') && isAl(x)) g -= 6;
    if (b == '.' && isLo(x)) g -= 4;
    if (isAl(b) && isDg(x)) g -= 4;
    if (isDg(b) && isAl(x)) g -= 2;
    return g;
}
float wrapJ(const char* prev, int W, const char* nx, int nl) {
    int e = W; while (e > 0 && prev[e - 1] == ' ') e--;
    int s = W - e; if (e < 2) return -20;
    char x = nx[0], y = nl > 1 ? nx[1] : 0; float g = 0;
    if (prev[e - 1] == '-' && isAl(prev[e - 2])) {
        char b = prev[e - 2], a = e >= 3 ? prev[e - 3] : 0;
        g = junc(a, b, x, y);
        if (!isLo(x)) g -= 5;
        if (s > 0) g -= 5; else g += 1.5f;
    } else {
        char a = prev[e - 1];
        if (s == 0) { char a2 = prev[e - 2]; int ca = cls(a), c2 = cls(a2); if (ca < 26 && c2 < 27) g += WJ * (lp3[c2][ca][26] - lp1[26]); }
        g += junc(a, ' ', x, y);
        int L = 0; while (L < nl && nx[L] != ' ') L++;
        if (L < nl && L + 1 <= s) g -= 12;
    }
    return g;
}
const int CH = 44, CW = 192, SR0 = 15, SC0 = 66;
struct Span { short dr, c1, len, tr; char allsp; string txt; };
struct Frag { vector<array<int, 3>> ps; vector<Span> sp; int h, w, cells; bool alive; int key; };
vector<Frag> fr;
struct Reg { char g[CH][CW]; int minr, maxr, minc, maxc, E; int ev[CW]; vector<array<int, 3>> pl; };
float C0 = 60.f, PRE = -1.0f;
float runCheck2(const char* row, int c1, int c2, int E) {
    float g = 0; int p = c1; while (isAl(row[p - 1])) p--;
    while (p <= c2) {
        if (!isAl(row[p])) { p++; continue; }
        int s0 = p, e = p; while (isAl(row[e])) e++;
        bool rel = (s0 < c1) || (e > c2) || (s0 == c1 && row[c1 - 1]) || (e == c2 && (row[c2] || (E && c2 == E)));
        if (rel) {
            bool lb = row[s0 - 1] != 0, rb;
            if (E && e == E) rb = true; else { char ch = row[e]; rb = ch != 0 && !(ch == '-' && (E ? e == E - 1 : row[e + 1] == 0)); }
            bool cross = (s0 < c1 && e > c1) || (s0 < c2 && e > c2);
            g += runScore(row + s0, e - s0, lb, rb, cross);
        }
        p = e + 1;
    }
    return g;
}
void buildSpans(Frag& F) {
    int r0 = 1e9, c0 = 1e9, r1 = -1e9, c1 = -1e9; F.cells = 0;
    for (auto& a : F.ps) { r0 = min(r0, a[1]); c0 = min(c0, a[2]); }
    map<int, vector<pair<int, int>>> rows;
    for (auto& a : F.ps) { a[1] -= r0; a[2] -= c0; const Piece& P = pc[a[0]]; r1 = max(r1, a[1] + P.h); c1 = max(c1, a[2] + P.w); F.cells += P.h * P.w;
        for (int j = 0; j < P.h; j++) rows[a[1] + j].push_back({a[2], a[0] * 16 + j}); }
    F.h = r1; F.w = c1; F.sp.clear();
    for (auto& kv : rows) {
        auto v = kv.second; sort(v.begin(), v.end()); size_t i = 0;
        while (i < v.size()) {
            Span sp; sp.dr = kv.first; sp.c1 = v[i].first; int cur = v[i].first;
            while (i < v.size() && v[i].first == cur) { int p = v[i].second / 16, j = v[i].second % 16; sp.txt += pc[p].rows[j]; cur += pc[p].w; i++; }
            sp.len = sp.txt.size(); int t = 0; while (t < sp.len && sp.txt[sp.len - 1 - t] == ' ') t++; sp.tr = t; sp.allsp = (t == sp.len);
            F.sp.push_back(sp);
        }
    }
}
// score fragment G with origin at canvas (r0,c0); 'first' = span index evaluated first (prefilter)
bool scoreFragAt(const Reg& R, const Frag& G, int r0, int c0, int first, float& S) {
    if (r0 < 1 || r0 + G.h >= CH - 1 || c0 < 5 || c0 + G.w >= CW - 5) return false;
    if (max(R.maxr, r0 + G.h) - min(R.minr, r0) > maxH || max(R.maxc, c0 + G.w) - min(R.minc, c0) > maxWg) return false;
    if (R.E && c0 + G.w > R.E) return false;
    float g = 0; int nadj = 0; char buf[CW]; int ns = G.sp.size();
    for (int q = 0; q < ns; q++) {
        int k = q == 0 ? first : (q <= first ? q - 1 : q); const Span& sp = G.sp[k];
        int rr = r0 + sp.dr, cc = c0 + sp.c1, w = sp.len, c2 = cc + w; const char* row = R.g[rr]; const char* t = sp.txt.data();
        for (int i = cc; i < c2; i++) if (row[i]) return false;
        char b = row[cc - 1], R1 = row[c2];
        if (minPW >= 3) { if (!b && (row[cc - 2] || row[cc - 3])) return false; if (!R1 && (row[c2 + 1] || row[c2 + 2])) return false; if (R.E && R.E - c2 > 0 && R.E - c2 < 3) return false; }
        int tr = sp.tr;
        if (tr >= 2) { if (R1) return false; if (R.E && R.E != c2) return false; }
        char x = t[0], y = w > 1 ? t[1] : 0; float gs = 0;
        if (b) {
            nadj++; char a = row[cc - 2];
            if (b == ' ') { if (x == ' ') { if (!sp.allsp) return false; } else { if (a == ' ') return false; gs += junc(a, b, x, y); } }
            else gs += junc(a, b, x, y);
        }
        if (R1) {
            nadj++; char R2 = row[c2 + 1]; char lb = t[w - 1], la = w > 1 ? t[w - 2] : b;
            if (lb == ' ' && R1 == ' ') return false;
            gs += junc(la, lb, R1, R2);
        }
        if (b || R1) { memcpy(buf, row, CW); memcpy(buf + cc, t, w); gs += runCheck2(buf, cc, c2, R.E); }
        if (R.E && c2 == R.E && tr == 0 && w >= 2 && t[w - 1] == '-' && isAl(t[w - 2])) gs += 1.0f;
        if (q == 0 && gs < PRE) return false;
        g += gs;
    }
    if (!nadj) return false; S = g; return true;
}
void placeFrag(Reg& R, const Frag& G, int r0, int c0) {
    for (auto& a : G.ps) { const Piece& P = pc[a[0]]; int pr = r0 + a[1], pcx = c0 + a[2];
        for (int j = 0; j < P.h; j++) { memcpy(R.g[pr + j] + pcx, P.rows[j].data(), P.w); if (P.tr[j] >= 2) R.ev[pcx + P.w]++; }
        R.pl.push_back({a[0], pr, pcx}); }
    R.minr = min(R.minr, r0); R.maxr = max(R.maxr, r0 + G.h); R.minc = min(R.minc, c0); R.maxc = max(R.maxc, c0 + G.w);
    if (!R.E && minPW >= 3) { for (int c = R.maxc; c < CW; c++) if (R.ev[c] >= 2) { R.E = c; break; } }
}
// Store competitor mass independently of the winning placement. This avoids
// catastrophic cancellation when the winner is much stronger than its rivals.
inline double logAddMass(double a, double b) {
    if (!isfinite(a)) return b;
    if (!isfinite(b)) return a;
    double hi = max(a, b), lo = min(a, b);
    return hi + log1p(exp(lo - hi));
}
struct SlotB { float s1; double otherTop, otherMass; int g1, r1, c1, dup; };
inline void addOther(SlotB& b, double score, double multiplicity) {
    if (b.otherMass == 0) { b.otherTop = score; b.otherMass = multiplicity; }
    else if (score <= b.otherTop) b.otherMass += multiplicity * exp(score - b.otherTop);
    else { b.otherMass = b.otherMass * exp(b.otherTop - score) + multiplicity; b.otherTop = score; }
}
// grow region from seed fragment index f; returns number of merges
int growRegion(int f, float T) {
    static Reg R; static int sid[CH][CW][2];
    memset(R.g, 0, sizeof R.g); memset(R.ev, 0, sizeof R.ev); R.pl.clear(); R.E = 0;
    R.minr = 99; R.maxr = -99; R.minc = 999; R.maxc = -999;
    placeFrag(R, fr[f], SR0, SC0);
    vector<SlotB> sl; vector<array<int, 3>> sk; int merges = 0; int NFr = fr.size();
    while (true) {
        if (elapsed() > TLIM) break;
        sk.clear();
        for (int r = R.minr; r < R.maxr; r++) for (int c = R.minc - 1; c <= R.maxc; c++) {
            sid[r][c][0] = sid[r][c][1] = -1;
            if (R.g[r][c]) continue;
            if (R.g[r][c - 1]) { sid[r][c][0] = sk.size(); sk.push_back({r, c, 0}); }
            if (R.g[r][c + 1]) { sid[r][c][1] = sk.size(); sk.push_back({r, c, 1}); }
        }
        if (sk.empty()) break;
        sl.assign(sk.size(), SlotB{-1e9f, 0, 0, -1, 0, 0, 0});
        for (size_t si = 0; si < sk.size(); si++) {
            int rr = sk[si][0], c = sk[si][1], side = sk[si][2]; SlotB& B = sl[si];
            for (int gi = 0; gi < NFr; gi++) {
                if ((gi&31)==0 && elapsed()>TLIM) goto done_region;
                const Frag& G = fr[gi]; if (!G.alive || gi == f) continue;
                for (int k = 0; k < (int)G.sp.size(); k++) {
                    const Span& sp = G.sp[k];
                    int r0 = rr - sp.dr, c0 = side == 0 ? c - sp.c1 : c - (sp.c1 + sp.len) + 1; float S;
                    if (!scoreFragAt(R, G, r0, c0, k, S)) continue;
                    // Each span has a unique (dr,c1); for one slot a given
                    // fragment therefore generates each (r0,c0) only once.
                    // Identical-content fragments at the same winning pose
                    // are interchangeable, and excluded from competitor mass.
                    if (B.g1 < 0) {
                        B.s1 = S; B.g1 = gi; B.r1 = r0; B.c1 = c0; B.dup = 1;
                    } else if (S > B.s1) {
                        addOther(B, B.s1, B.dup);
                        B.s1 = S; B.g1 = gi; B.r1 = r0; B.c1 = c0; B.dup = 1;
                    } else if (S == B.s1 && fr[B.g1].key == G.key && B.r1 == r0 && B.c1 == c0) {
                        B.dup++;
                    } else {
                        addOther(B, S, 1);
                    }
                }
            }
        }
        int best = -1; float bestC = -1e9;
        for (size_t si = 0; si < sk.size(); si++) {
            SlotB& B = sl[si]; if (B.g1 < 0) continue;
            const Frag& G = fr[B.g1]; float conf = 1e9;
            for (const Span& sp : G.sp) {
                int rr = B.r1 + sp.dr, cc = B.c1 + sp.c1, c2 = cc + sp.len;
                for (int side = 0; side < 2; side++) {
                    int o = side == 0 ? sid[rr][cc][0] : sid[rr][c2 - 1][1];
                    if (side == 0 ? !R.g[rr][cc - 1] : !R.g[rr][c2]) continue;
                    if (o < 0) continue;
                    SlotB& O = sl[o];
                    bool same = O.g1 >= 0 && fr[O.g1].key == G.key && O.r1 == B.r1 && O.c1 == B.c1;
                    double others = O.otherMass > 0 ? O.otherTop + log(O.otherMass) : -numeric_limits<double>::infinity();
                    if (!same && O.g1 >= 0) others = logAddMass(others, (double)O.s1 + log((double)O.dup));
                    const double prior = C0 > 0 ? log((double)C0) : -numeric_limits<double>::infinity();
                    conf = min(conf, B.s1 - (float)logAddMass(others, prior));
                }
            }
            if (conf > bestC) { bestC = conf; best = si; }
        }
        if (best < 0 || bestC < T) break;
        SlotB& B = sl[best];
        placeFrag(R, fr[B.g1], B.r1, B.c1); fr[B.g1].alive = false; merges++;
    }
done_region:
    if (merges) { fr[f].ps = R.pl; buildSpans(fr[f]); fr[f].key = N + f; }
    return merges;
}
vector<vector<int>> solve(const vector<string>& raw) {
    T0 = chrono::steady_clock::now();
    initLM();
    N = (int)raw.size(); pc.assign(N, Piece());
    map<string, int> cidm;
    for (int i = 0; i < N; i++) {
        string s = raw[i]; while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ')) s.pop_back();
        Piece& P = pc[i]; string cur;
        for (char ch : s) { if (ch == '|') { P.rows.push_back(cur); cur.clear(); } else cur += (ch == '_' ? ' ' : ch); }
        P.rows.push_back(cur);
        size_t w = 0; for (auto& r : P.rows) w = max(w, r.size());
        if (w == 0) { w = 1; }
        for (auto& r : P.rows) r.resize(w, ' ');
        P.h = P.rows.size(); P.w = w; P.lead = true;
        for (auto& r : P.rows) { int t = 0; while (t < (int)w && r[w - 1 - t] == ' ') t++; P.tr.push_back(t); P.allsp.push_back(t == (int)w); if (r[0] == ' ') P.lead = false; }
        char f = P.rows[0][0]; P.up0 = isUp(f) || isDg(f) || f == '"' || f == '\'';
        auto it = cidm.find(s); if (it == cidm.end()) it = cidm.insert({s, (int)cidm.size()}).first; P.cid = it->second;
    }
    minPW = 99; for (auto& P : pc) minPW = min(minPW, P.w);
    fr.clear();
    for (int i = 0; i < N; i++) { if (pc[i].h > maxH || pc[i].w > maxWg) continue; Frag F; F.ps.push_back({i, 0, 0}); F.alive = true; F.key = pc[i].cid; buildSpans(F); fr.push_back(F); }
    static float TS[5] = {0.f, -1.f, -2.f, -3.f, -4.f};
#ifdef LOCAL
    if (getenv("TS")) { sscanf(getenv("TS"), "%f,%f,%f,%f,%f", TS, TS + 1, TS + 2, TS + 3, TS + 4); }
    if (getenv("C0")) C0 = atof(getenv("C0"));
#endif
    for (int pass = 0; pass < 12 && elapsed() < TLIM; pass++) {
        float T = TS[min(pass, 4)];
        vector<int> ord; for (int i = 0; i < (int)fr.size(); i++) if (fr[i].alive) ord.push_back(i);
        stable_sort(ord.begin(), ord.end(), [&](int a, int b) { return fr[a].cells > fr[b].cells; });
        int tm = 0;
        for (int f : ord) { if (!fr[f].alive) continue; if (elapsed() > TLIM) break; tm += growRegion(f, T); }
#ifdef LOCAL
        { int al = 0, mx = 0; for (auto& F : fr) if (F.alive) { al++; mx = max(mx, (int)F.ps.size()); } fprintf(stderr, "pass %d T %.1f merges %d alive %d maxsz %d t=%.2f\n", pass, T, tm, al, mx, elapsed()); }
#endif
    }
    vector<vector<int>> ans; int page = 0;
    for (auto& F : fr) if (F.alive && !F.ps.empty()) { for (auto& a : F.ps) ans.push_back({a[0], page, a[1], a[2]}); page++; }
    return ans;
}
} // namespace DL

/*
 * Complete the 'assemble' function below.
 *
 * The function is expected to return a 2D_INTEGER_ARRAY.
 * The function accepts STRING_ARRAY pieces as parameter.
 */

vector<vector<int>> assemble(vector<string> pieces) {
    return DL::solve(pieces);
}

int main()
{
    const char* op = getenv("OUTPUT_PATH");
    ofstream ffile;
    if (op) ffile.open(op);
    ostream& fout = op ? (ostream&)ffile : (ostream&)cout;

    string pieces_count_temp;
    getline(cin, pieces_count_temp);

    int pieces_count = stoi(ltrim(rtrim(pieces_count_temp)));

    vector<string> pieces(pieces_count);

    for (int i = 0; i < pieces_count; i++) {
        string pieces_item;
        getline(cin, pieces_item);

        pieces[i] = pieces_item;
    }

    vector<vector<int>> result = assemble(pieces);

    for (size_t i = 0; i < result.size(); i++) {
        for (size_t j = 0; j < result[i].size(); j++) {
            fout << result[i][j];

            if (j != result[i].size() - 1) {
                fout << " ";
            }
        }

        if (i != result.size() - 1) {
            fout << "\n";
        }
    }

    fout << "\n";

    if (op) ffile.close();

    return 0;
}

string ltrim(const string &str) {
    string s(str);
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char c) { return !isspace(c); }));
    return s;
}

string rtrim(const string &str) {
    string s(str);
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !isspace(c); }).base(), s.end());
    return s;
}
