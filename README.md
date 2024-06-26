# mgs-mc-hd-decrypter

A tool to decrypt Metal Gear, Metal Gear 2: Solid Snake, Metal Gear Solid 2: Sons of Liberty, and Metal Gear Solid 3: Snake Eater Master Collection (Steam and Switch) and HD Collection (Xbox 360 and PS3) save-games

Note: this tool also updates the custom integrity checksums.

## How to Use

This tool requires a command prompt that can run exe files such as the default windows command prompt.

1. In a command prompt window, navigate to the directory containing the .exe file. 
2. It is recommended to make a backup of any save file before trying to run it through this program in case something goes wrong.
3. Use the below instructions to run the program. The below instructions feature the command you need to run along with multiple "fill in the blank" options. It is important to note that it is recommended to transfer the individual file you with to decrypt or encrypt to the same directory as the exe. 
4. Make any edits you'd like to the decrypted file using any hex editor. Some useful addresses for each game are below. 
5. Repeat step 3 to re-encrypt the file.
6. Follow the output of the program in the console. Depending on any edits to the file it may be required to rename the save file before adding it back to steam/switch.


```
USAGE: ./mgs-mc-hd-decrypter [operation] [game] [platform] filename

OPERATIONS     Explanation:
 -d            Decrypt File
 -e            Encrypt File
 -c            Convert File

GAMES          Explanation:
 -0            Metal Gear Save File
 -1            Metal Gear 2: Solid Snake Save File
 -2            Metal Gear Solid 2: Sons of Liberty Save File
 -3            Metal Gear Solid 3: Snake Eater Save File

PLATFORMS      Explanation:
 -p            Save is from the Master Collection for PC (Steam)
 -s            Save is from the Master Collection for Switch
 -h            Save is from the HD Collection for Xbox 360 and PS3

* It is recommended for the file being operated on to be in the same directory as the program. (It is required for any operations involving HD collection saves)
```
## Disclaimer

This tool, nor the author is not responsible for any lost or damaged saves. Please make a backup of any saves before using this tool. 

When using Steam, ensure that the game is running when you modify its save folder otherwise Steam Cloud may re-add any deleted files upon launching the game and cause issues due to multiple saves being present in the same slot. If not running the game, be sure to delete any files Steam Cloud re-adds when launching the game before attempting to load a save.

## Gameplay Stats Editing

These values are confirmed to be working for the Master Collection, but given the addresses found should also work for the HD Collection.
Making changes to these values will NOT change how the save appears on the file select screen until you save again over that slot (i.e. time will not change until you save again).
Note: any value you add to these files for the Master Collection should be in little endian format. For example if you want to change kills to 300 (0x12C) you would write this value into the save file as 2C 01

### Metal Gear Solid 2

```
ADDRESS     SIZE (BYTES)    STAT
0x136       2               Continues
0x13A       2               Saves
0x13C       4               Time Played (In frames, 60 x seconds played)
0x144       2               Shots Fired
0x146       2               Alerts
0x148       2               Kills
0x14A       2               Damage Taken
0x1594      2               Rations Used
0x1598      2               Times Seen by Enemy (Not Alerts)
0x159A      2               Special Items/Radar Usage (0x0000 = Radar Off, No Special Items Used, 0x201X = Radar Off, Special Items Used, 0x0020 = Radar On, No Special Items Used, 0x203X = Radar On, Special Items Used)*
0x14        1               Difficulty (not sure if this actually changes things like enemy health or damage in the current playthrough, this was tested on a save about to start new game plus and it does effect damage values in new game plus and end game titles on the current playthrough) (0x0A = Very Easy, 0x14 = Easy, 0x1E = Normal, 0x28 = Hard, 0x32 = Extreme, 0x3C = European Extreme)

* X refers to the special items used. Add the values of the special items used to get X. (Stealth Camo = 0x1, Infinity Bandana/Wig = 0x2, Wig A (02 Wig) = 0x4, Wig B (Grip Wig) = 0x8). All items used adds up to 0xF. Just Infinite Ammo and Stealth Camo is 0x7.
```

### Metal Gear Solid 3

```
ADDRESS     SIZE (BYTES)    STAT
0x34        2               Continues
0x36        2               Saves Made
0x38        2               Alert Phases
0x3A        2               Humans Killed
0x3D        1               Special Items (0x00 = Not Used, 0x01 = Stealth Camo, 0x02 = Inf Face Paint, 0x04 = EZ Gun, 0x05 = Camo and EZ Gun, 0x06 = Inf. Face Paint and EZ Gun, 0x03 Stealth Camo and Inf. Face Paint, 0x07 All 3 Items)
0x3F        1               Plants & Animals Captured (No clue if this impacts the markhor achievement or unlocks the EZ Gun if changed)
0x40        2               Times Seriously Injured
0x42        2               Damage Taken (Between lifebars, once this value reaches a certain point it resets to 0 and the lifebar count increases)
0x44        2               Damage Taken (Lifebars) (This is what shows up on the end screen)
0x46        2               Meals Eaten 
0x4C        4               Time Played (In frames, 60 x seconds played)
0x5A8       2               LF Med used
0x06        1               Difficulty (not sure if this actually changes things like enemy health or damage in the current playthrough, this was tested on a save about to start new game plus and it does effect damage values in new game plus and end game titles on the current playthrough) (0x0A = Very Easy, 0x14 = Easy, 0x1E = Normal, 0x28 = Hard, 0x32 = Extreme, 0x3C = European Extreme)
```

## Convert Operation Instructions

The behavior of the "convert file" (-c) operation differs based on the game selected. However, one common feature held between all conversions is that the selected platform is the platform the save will be converted to if possible.

### Metal Gear and Metal Gear 2

For these games the convert file operation will convert HD collection versions of Metal Gear and Metal Gear 2 saves to be compatible with the Master Collection. 

HD Collection saves consist of a DATA.BIN file and a MASTER.BIN file whereas Master Collection saves consist of 1 file with 29 characters as a part of its name.

To convert from HD to Master Collection, copy the contents of DATA.BIN into a valid Master Collection save file, they should be the same size. After that run the program and rename the file to whatever the program outputs as the new filename.
To convert from Master to HD Collection, copy the contents of the Master Collection save file into a DATA.BIN file from a valid HD Collection save. Place the MASTER.BIN file associated with that save next to the DATA.BIN file in the same directory and run the program.
There is no conversion required to go between PC and Switch for the Master Collection with these games, the files can just be copied as is.

NOTE: The information on the start screen when selecting the file will be inaccurate, but will correct itself once you load the save and save over the slot.

### Metal Gear Solid 2

For Metal Gear Solid 2, the convert file operation will convert Master Collection saves between PC and Switch. 

To perform the conversion, simply run the program and rename the file to whatever the program outputs as the new filename. From here you can replace the 29KB save file inside of a save slot folder on the platform you chose. You do not need to delete any other files.

Converting between HD and Master Collection for this game is a complex process given the size and numerous data types present in the save file. This tool does not support this type of conversion. 

### Metal Gear Solid 3

The convert file operation does not work with Metal Gear Solid 3. These saves can just be copied between PC and Switch with no modification required.

Converting between HD and Master Collection for this game is a complex process given the size and numerous data types present in the save file. This tool does not support this type of conversion. 

### Metal Gear Solid

This game is not supported by this tool, but it is possible to port saves from the PS1 versions of MGS, MGS: VR Missions, MGS: Special Missions, and MGS: Integral to the Master Collection. The Master Collection save is essentially multiple PS1 memory card files (.mcd files) placed back-to-back. If you have a .mcd or .mcr file with MGS save data, you can search for the serial number of the game (i.e. SLUS-00957 for VR Missions) inside the Master Collection file and scroll down until you find the save block. Each block is 0x2000 bytes exactly. When you find the block you wish to copy in your .mcd file (or have an entire .mcr file which is just 1 block) copy is over the 0x2000 byte block in the Master Collection file. The save should load fine when you load up the game.

To convert from PC to Switch for MGS1, delete the first 8 bytes and delete the last 0x194 bytes in the file.

To convert from Switch to PC do the reverse, copy the first 8 bytes from an existing PC file to the start of the Switch file and do the same for the last 0x194 bytes of a PC file.

## Master Collection Encryption and Decryption Instructions

Regardless of platform, the only file that is intended to be used with this program is one with gibberish as its filename and has a size of 29 KB for MGS2 and 19 KB for MGS3. 

This is the only important file for determining game progress in each save slots, leave any other files as they are. Once this file has been decrypted with the tool, edited, and encrypted again, the tool will instruct you to rename the file to a new name.
Once the file is renamed it can replace its previous version in your save slot and should load fine depending on any changes made in editing. 


### Credits

This tool is based on the MGS PS3 Save Decrypter - (c) 2021 by Bucanero and the original XBOX MGS 2/3 - SecFixers by Philymaster
