# mgs-mc-hd-decrypter

A tool to decrypt Metal Gear Solid 2 (Sons of Liberty) and Metal Gear Solid 3 (Snake Eater) Master Collection (Steam and Switch) and HD Collection (Xbox 360 and PS3) save-games

Note: this tool also updates the custom integrity checksums.

```
USAGE: ./mgs-mc-hd-decrypter [-2 or -3] [-p or -s or -h] [-d or -e] filename

OPTIONS        Explanation:
 -2            Save is for MGS2
 -3            Save is for MGS3
 -p            Save is from the Master Collection for PC
 -s            Save is from the Master Collection for Switch
 -h            Save is from the HD Collection for Xbox 360 and PS3
 -d            Decrypt File
 -e            Encrypt File
 
```

### Master Collection Instructions

Regardless of platform, the only file that is intended to be used with this program is one with gibberish as its filename and has a size of 29 KB for MGS2 and 19 KB for MGS3. 

This is the only important file for determining game progress in each save slots, leave any other files as they are. Once this file has been derypted with the tool, edited, and encrypted again, the tool will instruct you to rename the file to a new name.
Once the file is renamed it can replace its previous version in your save slot and should load fine depending on any changes made in editing. 

### Master Collection Save Transfers Between Platforms

Encrypting a Master Collection save file will make it compatible with whatever platform you choose. For example, decrypting a PC save using the pc option and then encrypting it with the switch option will cause the save to work on the switch. 

Note: it is probably best to replace an existing save file (i.e. create a new save file and replace it with the edited one).



### Credits

This tool is based on the MGS PS3 Save Decrypter - (c) 2021 by Bucanero and the original XBOX MGS 2/3 - SecFixers by Philymaster
