typedef unsigned long long u64;
namespace DL {
const int K = 27, MAXH = 14, GW = 64, NWD = 64, MAXPL = 320;
float lp1[K], lp2[K][K], lp3[K][K][K];
int minW = 40, maxWg = 60, minH = 10, maxH = 14, minPW = 3;
double TLIM = 1.6;
float LINK_T = 6.0f, LINK_D = 3.0f, WJ = 0.0f;
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
// WikiText-2 TRAIN, quantized conditional character probabilities.
static const char* NGRAM_DATA=
"ACoBRgE6ATYBJQE/AUIBMwEtAWYBVAE2AT4BLQEtAUIBdAEvAS8BKgE+AUwBRAFoAUYBcQEcAXEBQwE0ATQBXwFJAT8BZAE3AWUBSgElATQBGwF4AUEBiAEj"
"ASUBIQFGAUIBUgFtAT0BZQEnASMBTAFZAWwBFAGBAYUBbAEuAVcBoQEnAWMBbwEkAYUCKQE+AVsBJQF8AW8CIwI+ASABgAE+AXUBHwGOAX8BHwEuAjIBMgFt"
"AX0BGwF/AXEBMQFRAScBOAGfAYgBsQFSAZEBMwE1AX8BbQFNAR4BawFVAWEBJwFtAX0BVgFaAWUBNwGCAXUBQQE7AXABNwFYAWIBrgFLAaYBCQEwAV0BOAEl"
"AT0BTAFNAWsBRwGCAWQBNgE9ASgBVgFIAWIBIAEpATwBXwFBAUwBSgFLAXoBEgEwAZoBhQGqASgBMwGNAiMBmAGwAToBdQF8ARwBiAIoAWMBMwE6ApABmgFn"
"AhEBJQFtAZQBWwEfAYMBSgEnAS0BqQGQAToBZgE5ATIBiAIsAT0BUQExAa0BbAGpAUwBgwERASEBcQGAAWoBCwGJAYkBkwEjAo0BXgFiAVUBKwGHAYMBQgFh"
"AUEBRAGQAWABqQFXAakBIAE2AUwBKwE4ATMBRAE5AXwBYAGUAVYBMAE4ARQBKgFJAW8BNAEkASIBYAE4AZsBWQGUAVYBQgEYBBwBiQGJAYgBMAKKAYgBawIU"
"AYkCWwGIAWkBFgGFBUEBMAFsAZYBaQEWAW8BUAFKASABmwFoAUMBPwEvAT4BagJKASoBbAFJAX4BXAJFAhMBIQFNAWkBMgEdAU4BagF2ASIBrgFaASMBSQF4"
"ASkBWQJoATcBOwE7AVIBagGxASkBlQEdARsBMwFbAXQBGAF6AYIBhgElAZMBhQFkATcBYAEkASsBsgFqATwBcAE4AYUBgQGqAUcCHQEyAXMBNAEeASoBUwEj"
"AWkBNAFrAVQBUgFgAUcBNAF6AYMBawEwASUBRwFXAXABkQFKAXEBFAFLAU0BQgE+AVsBIwFMAV0BTAFtAVABNAEtARsBPgE7AZwBIAE4ATQBKQE9ATYBYQFV"
"AXgBIgEiAXIBdAFxARwBdgFzATkBKwGiAZABJQFVAX0BIAEwAh4BPQEzATMBrQGDAlwCLAFcAnoCRQFnAmYBRgV5A2MEAQY/AScBVwFCATkBGQFVAUcBaAEl"
"AZkBRgFGAT4BPAElAVcBiQFCAS8BMAFBAU4BZAGtATkBggEbATwBZgE/AX0BJAFjAYcBMwErAa8BWQFOAVMBawEvAT0BaAFvATQBIAE4AYUBYgHBAVUBlgEO"
"ATEBbwFbAZEBIgF4AY0BEgElAaQBmQFLAWYBbgEmAYgBxgE0ATkBQwE+AXgBSgG7AUIBdQEbATQBOQEwATcBMQFYATEBewE4AXcBXQEoATABIAFhATQBoAEe"
"ASEBJAGWAWwBhgFwAWoBaQFAASUCkwFoAQgBmAGjAYIBFwKTAYQCawEwAXwCbAF2AX8BZwGeA0wCRgEXAYEBcwFnAR0BcwGUASABHAJ1AU0BcwEyASUBfAI8"
"AUMBZwF7AngCaQIpASkBXgEsAYIBJgFNAYcBQwEgA24CfgE8ARsBegKFARwBQAFoAWwBaQFRAhoBPwFlAVABWQEsAXABZQGTAUMChgFAAUcBTwE4AUYCUAE2"
"AVIBXQGcAWYBmAGeAXkBBQEeAWUBYQIRAnMBNgEhAWwBcAFEAWMBZwEnAVwCWQJcAUQBZgFhAXkBOgE0ASABIgExAS8BOAE8ATIBQQEzASwBTgFQATwBNQE+"
"ASsBNAFqATcBKQEdAUgBTQEuAX0BVgFzAZ0cMgE4AzMEJgMQAxQDPQE4AjoEPQNDAiYCHgMdATICGQFNBU0BNgIXAU0ERwJKATUCSwEtARoEJQQ4AjoCTAFA"
"AT0CPwExAz0CDwwSAxwMIgUxAR4MKwIGBzcBGQQMAj8CMQU6ATADLwMpBxgaEAQoAkcBOwUkATQBDAQgATIBOQc8DwQNMwQJBBwSIgFBATsCUAExAUkDKgJJ"
"ARcBSgI8AVgCVQEmATMBVgFRAVsCTwIQASsBNgMNBCkEPQIwASUDOQgfAT4CMgEMAT8CNgI6AkcDPQFEBDkBLgFMBDgBZAEaHCgEIwMpAS8GMAERAisBNwEk"
"BzEcLwFMATUBJwEfAjQCLQI2ATABOQFCAT0BXAIxATIBIQJVAy0CIAFYAlgCKgNDATkCRwQ2AVUDKAElAUkGBwFPAkQCFwMxARgDUQFdAj8DTAFKASwBNwYS"
"BC8DHAUpAjAEKAEQASAHNgEmBAkEGwYuCi4DDA0pBS4IFgkQEhMBQAQdBC8KIggICQwTLQEyASkBMgFLAS4BOQE1ATYBTwFRATIBLAE1ATwBKwIuASEBKAFR"
"AUABMAJOBDQBIgFACCECHwQhAR4BHAU6AjsFFTEFHDcCJgE2ASQBMQEvAUIBMQMuAkIEGAE0ASoFSQIeUjIDNgMxBRcCIQEsAyEBLgEdDAEyJAQNBBUGLQYz"
"BCo5KgMpCDYBPwEwASUDIQE+ASABGQE9ATYBPwE4ORsEIgQQBhcGKAsQCigFHwcWH0AFIQM3ASEFIwErAQ1zABwiEiEMPANQBRcBJgEcAi4CIwEtASMBLQFJ"
"BTYcIwQRChQGGyJIAyYBTAQ3AzgBOAEhAjgCLAEeCA43GQQhBCQFRgEtAzcDNQYPARgDKwEdATQGMQIqAjACOgEsARwCOSUeBC8DNgciCAoBGAQbBCsGIAYQ"
"IQQcPgZOAlUDKwEWARIBSwFGAiMBSQFFASQBOgU/MAAHIgQRBBMGHQYtIQEBQwQbBBEDRAMhAzkBLgItBhwMEgEvAygCGAEhASNbFRgJHCEBMwEnATYBOgEx"
"AToBNwEtAzwBMwItAS4CMwEmASkCOwExDT4DNQEjASMEJAIhAT0BMgMZAiMcBh8VBA8DIRA0AUgBLwEhAUkBNgFUBTgBPQEoAUIBNAIbASIBQQI3BRwgARcN"
"GygCMAEyASsBMQFGBxUBOAMvAR0BJwFHAS1ADRQSCwgwLgIyBzIBHgEbATsBPgIzAj4BMAIdBCIwAAcUBBMEHwYeBishARwrAT0BEgImBzIBPQUUATEBPAc+"
"AQ4EFRcKCBI5HA8GHCIBKQE0ATsBQAExAUYBMwEoAVABUwE/ATgBPQEtATgCOwEsARoBSQFSATACYQUtASYDPQQ2AS8BMwEyAlMCFwEbAR4BUAE7BTMBHgQe"
"DRIDHAclBCQDMgEkAkIBPgMZAzoCEgEzB1UEQgJRAjgGVQNZAVgCSgRgAgIEJwcqATQBOwESAjUCKgE2ASUHHAUVASADKwM0AxgFJwEuBioBEwQmBBUGLAMu"
"AykENAI4ARkIDRkgBxcEDwIsAiUzGggIASwDLgEdATYDJgMdAUQCLQE+AzABOwJJAyYCIAEhARsDGwQqBikBIQMyBkICHwFBAioBKQEvAjQCPwFdBEYBRwNS"
"ATIBEQFMAVwFFQwiAioCFQIcAy8GGgEkBCgDNgEdAygDJQMlAhgBNAYvFQAHLgFRAUkBZQEgAUMBSwFkAS0DUwE/ATIBRgFYAkMBIwE6Aj0BWgI+Ag4CXAE6"
"AiwDUAE0BlEBOQMjARwBQwYKAS4EIgMqASYFSAFFAy8BLwEoATICJgI5AhYNHgUOAiEILgQGBBoGMw0wBCYDNwEtCiIICAEkAiQCLwQlBxQEGAE6BisFLAo2"
"DAIbBgEmATIBLAE2ATgBLgE8ATYBLwFRAVABOQEzATkBKwExAWUBMwElASQBQgFLAS4CUwUZBiUDJQEgASQEJwEwASgCMwUuUiEBNgEoATIBKwctATYBJwQY"
"AS8EMwQkBRAEFAYwDBk5GwImAT0BMgUjAhwEFQFAATwCMTwfBBgEHwYWBiEEJztLCSsDOAMEAyQ9JQQgBCYGBwZBIQQFBwM1ExQMDwIbBBoBJAEujwEmATkB"
"MQE8AT0BOQFCATEBNQFJAUwBPQEyAUABOwE2AjwBLQERAVEBTAE2AlQGMAM/Ah4DKwEYARwEJgE+ASgCNgQ6ATlFAxAoCDwBQAEeATcDHgEiAS4HESAKF0AE"
"NQc3AzIFDAM2BBEBLwIqAjsHOQEyAQ8BIwM3AS0BOgIjPBQEEAQbBikKJx4hBBYEFhIVASUDIwUxAycCKwEoAywCLQIdBSA3DwQTBDQGFicCCAkHEQ0jBBcE"
"HgMkAiMELAEeaRoLBRwhATcBNAE8AT8BMgFEATMBKwFTAVUBQQE5AUYBKwE3AjwBLAEWAUgBTwEvAlIESwIiAUUEPAM3ATMBIwI+AiUBKAESAicBUwRMSA0K"
"QAM/AV0EPANEAUUBOgFnAyUBQAFjBT4CBVNLAR4CQQIvBSUBLwEjAisCNwERATYHUjsRFBMHER8cCiIICwRDCCkBLAE5AS0BOAIjASoBNwEYAiAEIjsQBBwG"
"DicCBScELwooCAYNHQEhBBABICQIBBJEHAgMHB0BNgErATwBPAE3AUIBMAEuAVIBSwE+ATQBPwEsATcCOgErAR4BUAFMAS4LPgUVAS4BFAREAiUHGAEvBBME"
"HwMXBjIDHwcXBCYDHAEtAi4BRwNABDUBLQE4BhkBMAM4AQsCNAI5Cj4CPQYWA0IBIgI4Bi0CIwQqAREBMAIxASsELQE0BBsBHAMUBjcFJwU5AiMBKgQ9AkMB"
"DQEiBRgERAM1BjokGhIFIAMXMgMpAR4EJgMVASsCPwRBATYFLgIhASABQgMSBCYENQI0ASEDMwI+Bh4BNgIzATgBLAFHARMBZAE7AWMBUgFfAkoBVAQ0ASwB"
"SQFNBRIEQQNSBUkCAwQ9AVQCLwY6ASQEMQcpBCkDHgEmBxAVAAc5AjYBLQEYAkkCNQM+AT8CPAFFAxkBOAcVAUQCPAIvAyQBLwM+AUQCNQFLAlYBMQEeBw0B"
"LgJJAiQDFwEeA0UDRgQoATYBPwQmAVsBGw0GBhoJKQQGBBoGQywcDxwHChwZBAcXIAE0ATEBMwIyAjEBKwM5ATEBOwE2ATgCMAEmASkDJwcbChsBFQIhXwwP"
"IwUiYxGcARYBIQkVAS0CKgQvjQEsCBkCFgQkARi4ARgEHwkkVRsBOAcxAiUEKAErASkFNAISXR8CCAY3BzJPDB0BBSIKBF4KKSsMAqIBChwgATIBMwE7AUAB"
"MgFFATQBJwNBATkBQAEkATUCOAErASEDLAY5ASgBQANBAjMCTAJBARkCTwIgASwBHAFEAVIBPwIjAjgFJgogBggmKQM8ASkJMgE5CAUBIwFSATMBJQFEAUQB"
"OQJWBDkBNQFQAzEBIwEzAVgBOAFSAU4BPgISGwM3KgFIAScBRQEpATMBLQQxAjkBHAEvAUsDIgEiAi8EQgFDHBsaEAEuAU4DHQQjBiUENQJJBB0CEQUhCiEE"
"JAgLHEYBTAEmBCkGRwEcAS4BLQItASUBNgE5ATYBGgI7AjwIGhMUIAsbNQoPBToHCwEyBCUDGgEfBjYENAIsBDICEwIlAhABIQgmATUFKAErCCEEBBcCPkkE"
"RgVIAUcIARwhATIBLAE0ATsBMgE9ATUBLQFPAU8BOwE2AUEBKgEwAjQBJgElAUsBSAEvBz4BLwMuAU8BKwE7ATQBKAIVBCABMwEkBE0BLgI9ASgECAQmAykD"
"MwM4DA80OgMlAUoHRAE4ARcEJAEnAS4EUAMTUkMCKQEwAT4CPAUYAhQEQwEjASgEOAM1UhoEEwQfBh4GIgYsHy8IPwFHARcBQQMZAR4BKwEnASYGGwQmAzMB"
"JgMYAyEDLgE3ASgBOQY3OyAWAx4hCSIBMwEcBCwBDnQEHCEBMwEyATsBQQEzAUUBLwEqAU4CQAE3AUMBLAE7AjwBKgEZAUgCLAZDAjgDMwJHAxMBJAEyBDIB"
"PQEYAjsFJxxJBAoDJwEqAx8DMwMzAjkFPwJBAUAEJwQxA1oDPgNQATcCRwRgAgQBNQJCAR0BPgJMAksDPQFNAUABVAMmASQBOAFUAUEBJwFCAUACEwEdBB0E"
"IQMXAxoNSANQAS0EQgMzA1sDTwExAUMBRwYEAQ8bLAImAjMBNwE2BjoBFwEzAkYCIQEcAjMCRgI4AUEVDgsWBCQKIwgQCSQQBQcCFyIEEgQWBiEGMwQwAjAJ"
"QgNDATMBLwQcAUIBEwEyAS0BIQQkTw4DSAJJAisBRQI2ASEDTQM+AToEHgE5BgwBKwQdAzkBIQM9Ay0DKAEnAjUERAITAR4CLgIbCBMGJAEqCCYEDgQWBhdP"
"Mg4DHB8BNAEyAToBOQE1AUYBNAEtAUMBVgE/ATYBQgErATgCOwErARkBSQFQATACWgUiARIIHwYiASUBIAgjBBMEKwEuAiYHHQgqARgCJAImBCACGQkkAS4H"
"MgQXBCsGOAQwAhoEKgIWDRgGEAEeDFABLw5GBwEBKwQhBBwFKAQVBx8CMQ4GECUBKwgqAgoEMAEiDQEbGAQlCiYICwEzAx4BIwQgAxoDIgQ/AjEBNgM9AiYB"
"LQE4AxoELQQhAkABHgNFCBIBMQI+ATQBKQFKASsCQwM+Ak4BSgQiAS0BYgFOA00CCwQZBxgBIgEvASwCMgIqATwBJAcwBRIDOAEkAykBNQIxASoDMQE3ASgG"
"Ihw6AVgBPAEpASUCQAIsAjIBPQEqAT4BTwFLAkkBNwEhA1QCNQITATYEFwQjAz8DQwE2Ax4BFActASwBQAMdAxMBLgY8BDwBMQcVAkkBUQQgBSYCGAI5Ah8B"
"IwEZBzsBNwQDBB4TNQQYBCUDMQIUBTEIFwkPEhEBFwQWDiUIFxwkASsBLgEzATkBNAE9AS8BMwFQAUoBOwEyAUIBOAEwAjIBKQEcAUsBUQEyAlEFLARBAikD"
"MgIfAjwCDwEpASgBQgRFAkNSIgIhASoBRgdAAiEBMwMRAT0BMgcsNyIEHwQXBh4KHwIgATsCGgE6ASoHMgIgASsDKAEgARw+CgQUBCsGKwY9BDoHAzY2BCoD"
"JAInATkBKwIUARoBOgFBAjEESQE4BBAELQMlAxQDIiUzBBIEHAYMEScWBQEqBBQEFwY2BC8CJwYbAhMKFwI0BCABLQEfcwUcIgEyATYBOgIxAjMBKwQ6AiMB"
"OwI7ASkBHAMooQQTBA8EFbUBVQFAASQBLwFRAToBPAIrAx8BNQEeAjUCPAE6AR4CPgFDAkEBUwEzARoEGwQZBh4KJQcTAwwBLQkzAzIHNwQbBCAGQwQcCA0B"
"JQFXASwBJgEuAUEBPAJQAWMCMgE5ATICOAFOAU8BIQE6AjoBTAJRAhUBHgQlCgsGJgceBAoEJgwuBjAcLAEyASIBOQEiAUwBLgRRATsBPgEeAS8BQAMqASAC"
"MARNAUQgGgQoCiAICgEwAxcBLgQaEBECMAEUBBkEJgYyBCkGLwIZAR8EGgQiBCoCNQQvCBABMwE6AToBMAJIAT4CVwFHAVEBMQEYASwBOgEsAk0BMgEwAR8B"
"NAEzAU4BPgI+BSIKDwMgASUkIwQVBBAGIAonA0YEKQM4ASwGNAUbAUsGCQErBCMDGwEfA0IBRQJFA0UBKgI6BDgCFAEuAhkCOQIxAikDKgEtAR4COwMdATcI"
"HwQPBA4TCj5DA0EPAhwjATMBLwE4ATsBMwFBATIBLQFNAVEBOwE0AUEBKwE0AjcBKAEdAUsBTwEvAlMEPgE4ATsCOQE1AiADJwEjASQCPAItATgBLgEvATcB"
"OAIuAi0BExslBCcDGwEnBhcDGAMqIisBUQEsAR4BOQcxAUEBKgI6AUUBJQEtATUBUQEzAkwBVAIXBRcQDCIqBBUEGgREAhoDRQNHBhkBOgE3ASwBJAFRAUwB"
"KQVDATMBHAEcAU8CSQEtASECNgI4AkYBTiAVBBgQIgIdAQ4EHwQeBiEKHwMNCB8GKQwYHjEBMQgrASMBEwFGAzUDIAFKBRsBIQQPAz0BHgMyAxsDMB4BBw4b"
"LAQcATsDFQYrBi8GEgEcBCIEKANKAVcCJwMiATsCOgRJAhIBMAElARwCMAE6ATQCPAMpAS8BMQIgAhwBNCgOBBU5JwEhASUFEQgYHB0BMwExATgBPgEwAUcB"
"NgEqAVcBUQE8ATcBPQEnATUBbgE3AS4BIQFIAVABKwJiBDABMAFMAk0BNAIkAi8BIAFPASICTwIfAUQBGwFOBEQCMgEEIgIvOQJAARYBRAJWAk8DNwExASsC"
"WgITAS4DWwJPAx43KQNrAQUENQNmAzsDPQFcAlgCYwJuAiMBNQImAjYBRgFQBTYBKQEeAQ8BWAJJATcBNgIsBUk3JQQLBDUQEAMVBAoXEQQLGEYBWQFVA00F"
"RgFDAS4BPAE9AiECSgE/AjgEBzcRBB8EHgYcBiQELAdDBEISAQEcBA8EKQMdAy8DNwc/Ai0BJQMjCUABNgQIATsBMiIBATMEEgQvBgkzLwQpCwMbDAEkATQB"
"MgE5AT4BNAFEATABKAFTAVIBPQE2AUEBJgE3AjoBLAEcAUkBUgEqAlEGLwM0BQ8CLgQTAicRMgIOBx0IIAE0AiICHQMVAToCLwE0CBQMEAI0AhAQLgIzBCMB"
"OAcyAh4FHAgOBgYWNQYnAQYNICQuATAIFAImBCQBIgEYPQ0BGgMXATABRQM8AyYIGwUvAigBKAEZAx8ILwI3AS4DNAgRAUECKwETATACMgIdAkIBSQI7BTcB"
"HAcuICkEMQYuARgDMggOHC8BRwEuAUkBGgFKAUQCHwUmATgBRgItATIBLQFMAT4DMAIkAToCQgIVAzkBIgM6BzoBGwFDBhUBRAJFAiEDHQEoBkQEOQFDAUAG"
"DKMBIxIaASoDKQ0xAxYCGwQcATUBHQctUlIDJggpATwBJAQRAS4IFVI5AiUBHwEfBycCHQEtAzgBHgEyAjmPASMGHgMTBiACKJUCAT9KAzMCOwQfAQcBMwUo"
"VDMBSgItASEEQgMnAjQEDgEoAy0FNjcvBBUEDAYaDz8BOAJDBigCHAU4AQhCDRQZIioOKAgEDTEBLQEwAw8DGwYWPwcGFScDvgEkATUBMAE2ATsBNAIxAS8D"
"PAE0Ai0BNQI5ASkBIgMsAi4BPQ4LBhcoDTQZARQKHWEbChgGFp0BBA4lBA4HFgMoAyFfFA0dCRIBAqIBIg4nBCkUFgIbBBMJJScLKw4DGQg5BhgCOAc1XwFE"
"FQQTBB8GIREYChkBEwwTKB8EGAMKCw8EGgsYKwkbIwsTBwsIBrYBIAExAS8BNAE9ATIBRAEzASwBTAFRAT0BNgFDASkBNgI1ASkBIAFJAVABLQJaFhJZJgMQ"
"DiQJF18JogEM0wI+ATcBPQFtATsBQAJJAykBQAESAj0CLAEmASwBQQFTAU4EGwEiBBIENAM4AyUDJgMjBBwCTwEbBC8DIAE1Ay4DDgMqAzoEUgJRASIEEQQZ"
"BiMDLQMlBEQCRwEgAkMBNgI8AUkCPAMrATIBGgI0AUgBPwEvAVABOgEkAhwBTwJIAScEKAQaAzIDEAMbAlIBNAZcARgEHgQqAzkDHAMYAyYGSQEUBBQEFgYh"
"BjMEUgJOA2EBTAJMAV4CTgNVAToBBwRIASABIAc9ARUEJgQyBhUGEwcjBB8DOwEUBCcBGgEyAzsDPgY8ARQEGAQXBhkGRARJAkwBEQJJAiAEHwRQAhgBVgUq"
"BEUCOwEdA0cBFgNQAS0GDwYrBE0CRgJJAUEBWwIKBkIBYgEaAjgCKwFeATgBOQE6AUkBYwNcARwEIQM8ATQDIwMdAxQDLRsCByIECANQASkGHwYwBk4BLgIy"
"Ah4DKAEpAlQBSgFIAWEBJAEwAVUDHQEnAlECRQIhAUMEOQMGAT4GGwM2A1ECQgJcAloLRAE8AgwCHQJCARYBRAc0ARoEFwQOBiIMQAETBB4DGgEYBiUDOA4N"
"FyIEDAoRBjULFAoWrggGYQaPAwlCCTsUBBgSDlEI3QEXBBASG8QCCbgEHgEfEiLmCQPcAQZ+CZcLA/gBHw4YASUHEGwQ/wQA1AMKXwKjBAJPEwgQUhdVHQos"
"ChMCEEQDbQgMEQgJtgEjATUBJwExAjADMAQzAiwBLAIxASQBKwMydCgOHgEfCAtdC7AEBRs9BEMKQQgBGw0gCdoCCwkaEhCHAQIcGwEyAS8CKQQnBiwBNQMp"
"ASgDMfcIDJsIEjsDJhcICgEgAyQXCSAEggEMCRUQCk0IRQwtAwUeBBYSEocBARwjCCULH3cMFw+BAgFjGQQVEhgFFxYLowEaAS8BMAE0ATsBNQI4ATEDOwE3"
"ATUBMwExAjEBLAEgAzNCADITDhsBIwgTOw0WFg4kAQUHKJ4BCHYTBA8oA5EBJwI1BSkBKgY1BC0BFAMvuwEHtAIJDxDmAgXkBAqQDA0OCoMDAr4BHxMf7gUL"
"Nw2hAQhfEmIWKQgbHQ8cByEIEJECAfQBBS4KiAEIKgaGEwU1Ag4CHAANCEMBgAIHMgTeJANUDxALpRIEhAEDJwXHDg+DEQB7Ar0BA4cYIRMV7g0TDwoFBzEL"
"VhU2EAQPJQGCCAafCwGCAQGFAQQ2AFAQCAm+AR8IJQYfBR8DK8QLGAQPXhMPFGUXAhCjAQ6lAQORARwHLAEpBigELAElAyYQBm4NBQ+bAwWZCwT4BQWgDw8D"
"EhEiCBEDDg4lARwCKw0XARVcClkYAgwEGw4AvA0MbRISEqEBCpUJBg4TaQgHGJ4BBXUALQTVAw8TD9UqBa0SASAE5wELXR4OHQgOAxgGGRIVOwWDASATH+cG"
"B5cLBCEC2QECagEZFxEWEAU2A9AgCOYBA8MXHA4gBR6bBgwPD4oDAc4eKwIXBDEBIAElBxcFFRQLAxwHIAcOERQEGDYiBg0DFyUXBBQEESAFBS0BHz4TGxEO"
"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"
;
// Quantized character model trained exclusively on public WikiText-2 TRAIN.
// No puzzle input, reference placement or validation text is embedded.
static vector<unsigned char> ngQ;
static const int ngBase[6]={0,0,27,756,20439,551880};
static void initNgram() {
    ngQ.assign(14900787,255);
    vector<unsigned char> bytes; unsigned acc=0; int bits=0;
    for(const char* s=NGRAM_DATA;*s;s++) {
        if(*s=='=') break;
        acc=(acc<<6)|dec64(*s); bits+=6;
        if(bits>=8) { bits-=8;bytes.push_back((acc>>bits)&255); }
    }
    unsigned key=0; size_t i=0;
    while(i<bytes.size()) {
        unsigned delta=0;int shift=0;
        do { unsigned v=bytes[i++];delta|=(v&127)<<shift;shift+=7;if(!(v&128))break; }while(i<bytes.size());
        if(i>=bytes.size())break;
        key+=delta;unsigned q=bytes[i++];
        if(key<ngQ.size())ngQ[key]=min(q,254u);
    }
}
static inline float ngProb(const char* row,int at,int lo) {
    int code=0,mul=1;
    float value=-3.5f;
    for(int n=1;n<=5 && at-n+1>=lo;n++) {
        char ch=row[at-n+1];if(!ch)break;
        int v=cls(ch);if(v>26)v=26;
        code+=v*mul;mul*=27;
        unsigned char q=ngQ[ngBase[n]+code];
        if(q!=255)value=-q*(1.0f/16);
    }
    return value;
}
static inline float ngSeam(const char* row,int seam,int end) {
    float gain=0;
    for(int p=seam;p<min(seam+4,end)&&row[p];p++)
        gain+=ngProb(row,p,max(0,p-4))-ngProb(row,p,seam);
    return gain;
}
struct HS { vector<u64> t; u64 mask;
    void init(int lg) { t.assign(1ULL << lg, 0); mask = (1ULL << lg) - 1; }
    inline void add(u64 h, int b) { h = (mix(h) & ~7ULL) | 4; u64 i = (h >> 3) & mask; while (t[i] && (t[i] & ~3ULL) != h) i = (i + 1) & mask; if (!t[i] || (int)(t[i] & 3) > b) t[i] = h | b; }
    inline int has(u64 h) const { h = (mix(h) & ~7ULL) | 4; u64 i = (h >> 3) & mask; while (t[i]) { if ((t[i] & ~3ULL) == h) return (int)(t[i] & 3) + 1; i = (i + 1) & mask; } return 0; } } HT;
const u64 SW = 0x1111, SP = 0x2222, SS = 0x3333, SI = 0x4444, FP = 1099511628211ULL;
inline u64 hstr(const char* s, int n, u64 salt) { u64 h = salt; for (int i = 0; i < n; i++) { h ^= (unsigned char)s[i]; h *= FP; } return h; }
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
    static double c1[K], c2[K][K], c3[K][K][K], st[K]; memset(c1, 0, sizeof c1); memset(c2, 0, sizeof c2); memset(c3, 0, sizeof c3); memset(st, 0, sizeof st);
    static const double BW[6] = {300, 40, 8, 2, 2, 2};
    auto isV = [](char c) { return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u'; };
    for (size_t wi = 0; wi < words.size(); wi++) {
        const string& w = words[wi]; int n = w.size(); double wt = BW[min(bk[wi], 5)];
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
            const string& s = forms[f]; int m = s.size(); double ww = f ? wt * 0.2 : wt;
            { u64 h = SP; for (int i = 0; i < m; i++) { h ^= (unsigned char)s[i]; h *= FP; HT.add(h, bq); } }
            for (int i = 1; i < m; i++) HT.add(hstr(s.data() + i, m - i, SS), bq);
            for (int i = 0; i < m; i++) { u64 h = SI; for (int j = i; j < m && j - i < 8; j++) { h ^= (unsigned char)s[j]; h *= FP; if (j - i >= 2) HT.add(h, bq); } }
            int q[40]; int ql = 0; q[ql++] = 26; for (int i = 0; i < m && ql < 38; i++) q[ql++] = s[i] - 'a'; q[ql++] = 26;
            st[q[1]] += ww;
            for (int i = 0; i < ql; i++) { c1[q[i]] += ww; if (i >= 1) c2[q[i - 1]][q[i]] += ww; if (i >= 2) c3[q[i - 2]][q[i - 1]][q[i]] += ww; }
        }
    }
    double T = 0, ST = 0; for (int i = 0; i < K; i++) { T += c1[i]; ST += st[i]; }
    for (int b = 0; b < 26; b++) { double tot = c2[b][26]; for (int y = 0; y < 26; y++) c3[b][26][y] = tot * st[y] / max(ST, 1.0); }
    double p1[K], p2[K][K];
    for (int i = 0; i < K; i++) { p1[i] = (c1[i] + 1) / (T + K); lp1[i] = log(p1[i]); }
    for (int a = 0; a < K; a++) { double s = 0; for (int b = 0; b < K; b++) s += c2[a][b]; for (int b = 0; b < K; b++) { p2[a][b] = (c2[a][b] + 5 * p1[b]) / (s + 5); lp2[a][b] = log(p2[a][b]); } }
    for (int a = 0; a < K; a++) for (int b = 0; b < K; b++) { double s = 0; for (int c = 0; c < K; c++) s += c3[a][b][c]; for (int c = 0; c < K; c++) lp3[a][b][c] = max(-9.0, log((c3[a][b][c] + 3 * p2[b][c]) / (s + 3))); }
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
#include "calib.inc"
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
        if (b || R1) { memcpy(buf, row, CW); memcpy(buf + cc, t, w); gs += runCheck2(buf, cc, c2, R.E); if(b) gs += 1.0f*ngSeam(buf,cc,c2); if(R1) gs += 1.0f*ngSeam(buf,c2,CW); }
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
    initLM(); initNgram();
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
