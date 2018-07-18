# YASS - Yet Another Sig Scanner


Original thread: https://www.unknowncheats.me/forum/anti-cheat-bypass/284848-yass-sig-scanner.html

Traditional sigscanning relies on static compiled code (same registers, same order of operations, etc) which can become unreliable when programs begin to add randomization, but retain the same code semantics.

Example 1: 
```
mov rdx, 0x1
mov rcx, 0x2
add rdx, rcx
```
Example 2:
```
mov rcx, 0x2
mov rdx, 0x1
add rdx, rcx
```
If you were to make a signature for Example 1, it would look something like this:
```
"\x48\xC7\xC2\x01\x00\x00\x00\x48\xC7\xC1\x02\x00\x00\x00\x48\x01\xCA", "xxxx????xxxx????xxx"
```
Unfortunately, this would fail on Example 2, since the actual assembly instructions are swapped. Semantically, the two examples have the same functionality though.

**Initial solution**: Create all permutations of a particular signature, and scan for every permutation until you find a non-null address. <br>
**Analysis**: Given a string of size N, there would be N! permutations of that string. Even a string of size 16 would result in pretty large number of permutations...2.092279e+13 to be exact. Yeah...not good. 

**YASS solution**: Create all permutations of instructions of a particular signature, and scan for every permutation until you find a non-null address. <br>
**Analysis**: Most signatures won't be longer than a few instructions, so this brings the number of permutations down by a lot. I think this is valid reasoning since any sane person would expect long signatures to break easily.

**Possible Improvements** <br>
Sometimes we are interested in grabbing an actual offset that is used within the instruction, but since the order of the instructions may change, the number of bytes to ignore changes as well.

Example:
```
mov rcx, 0x10
mov rbx, [rsp + 0x48]
```
Let's say we are interested in the offset 0x48 bytes from the stack pointer. In this simple assembly stub, that offset would live at the 12th byte. If the game were swap these two instructions, the offset would live at the 4th byte. You would have to add additional logic to figure out the correct number of bytes to ignore for a particular permutation.

**Caveats** <br>
It's possible that a particular permutation may bring you to an incorrect address if there exists some code that matches your permutation's opcodes. 

## How to use this library
1. Find the instructions that you are interested in pattern matching
2. Split the instructions into a vector where each element in the vector represents an individual instruction. <br>
Using Example 1: <br>
```
{ "\x48\xC7\xC2\x01\x00\x00\x00", "\x48\xC7\xC1\x02\x00\x00\x00", "\x48\x01\xCA" }
```
3. Split the masks as well
Using Example 1: <br>
```
{ "xxxx????", "xxxx????", "xxx" }
```
4. Construct a YASS object, and call Scan. The return value is non NULL if succeeded, NULL if failed
```
PatternScan::YASS yass({ "\x48\xC7\xC2\x01\x00\x00\x00", "\x48\xC7\xC1\x02\x00\x00\x00", "\x48\x01\xCA" }, { "xxxx????", "xxxx????", "xxx" });
yass.Scan(buffer, size); // buffer is the target byte array, size is the size of the buffer
```
