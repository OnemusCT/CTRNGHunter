# Chrono Trigger (PC) RNG Hunter

## Simple Commands
### list_rng

Lists the RNG values for a given seed. The list includes the raw RNG value in hex as well as the index for the battle RNG table.

#### Parameters

`-s --seed` - The Unix Time for the starting seed
`-n --num` - The number of RNG values to print for the seed

#### Example

```
> rng_hunter list_rng -s 1600000000 -n 10
   0: 0x0E4C, 0x5B
   1: 0x6EF6, 0x66
   2: 0x6ECA, 0x3A
   3: 0x0B48, 0x54
   4: 0x1D8E, 0xAC
   5: 0x3C70, 0xAD
   6: 0x77DA, 0x53
   7: 0x71D5, 0x48
   8: 0x61EA, 0x4D
   9: 0x0F8D, 0x9D
```

### list_crits

Lists the RNG values that result in a crit for a given threshold

#### Parameters

`-t --threshold` The threshold for a crit (or % chance for a crit, for example, by default Crono's value would be 10% crit"

#### Example

```
> rng_hunter list_crits -t 10
0x00 (0xB1): false
0x01 (0xCA): true
0x02 (0xEE): false
0x03 (0x6C): true
0x04 (0x5A): false
0x05 (0x71): false
0x06 (0x2E): false
0x07 (0x55): false
0x08 (0xD6): false
0x09 (0x00): true
...
0xFF (0x0C): false
```

### convert_to_seed

Converts a Date/Time string into a unix time seed.

#### Parameters

`-t --timestamp` The Date/Time string to convert, the expected format is the same as used in the CTManip application

For example, `TimeZones.ET, 14, 11, 2023, 17, 20, 52`, being TimeZone, day, month, year, hour, minute, second.

The accepted values for TimeZone are

```
TimeZones.ET
TimeZones.UTC
TimeZones.JST
TimeZones.GMT	
TimeZones.CEST
```

#### Example

```
> rng_hunter convert_to_seed -t "TimeZones.ET, 14, 11, 2023, 17, 20, 52"
1699996852
```

### convert_to_timestamp

Converts a unix time to a Date/Time string for use in CTManip. The opposite of convert_to_seed.

Always create a UTC Date/Time

#### Parameters

`-s --seed` - The unix time to convert

#### Example

```
> rng_hunter convert_to_timestamp -s 1699996852
TimeZones.UTC, 14, 11, 2023, 21, 20, 52
```

## Complex Commands

Complex commands take an input file that specifies the number of RNG affecting actions occur. 

The possible actions are:
```
new_game
load
room
portal
heal
battle
battle_with_crits
battle_with_rng
```

NOTE: Comments can be included using # to begin a comment, for example `battle # dragon tank` could be used to indicate the dragon tank battle. Additionally, lines that begin with # are ignored as well


### Basic Actions

Basic actions are only for accounting for RNG rolls that happen naturally in the game. The following are basic actions:

`new_game` accounts for the number of RNG rolls from selecting New Game to gaining control of Crono in his bedroom.

`load` accounts for the number of RNG rolls from loading a save file or bookmark

`room` accounts for the number of RNG rolls from a room transition. 

`portal` is currently only used for the initial portal transition from the fair to Truce Canyon at the beginning of the game. For some reason this uses a different number of RNG values compared to a normal porta.

`battle` accounts for the RNG roll to initialize a battle, but when searching for seeds no specific RNG value is searched for

`heal` accounts for the RNG rolls used to heal using a Tech outside of battle. Optionally takes a parameter to indicate the number of heals (example: `heal 5` if 5 heals are done)

### Advanced Actions

Advanced actions are used for searching for RNG values that match certain criteria.

#### battle_with_crits

`battle_with_crits` is used to find a battle that has critical hits

##### Parameters

The first parameter is a comma delimited list of crit thresholds. This should match the crit threshold for the party members based on their turn order. This list is repeated if additional rounds are included in the search.

For example, with a party of Frog, Crono, Robo, this parameter could be `23,10,10` if Frog goes first and their speeds are equal

If speeds are not equal and eventually the turn order changes a longer string of thresholds can be provided to account for this, for example, `23,10,10,23,10,10,23,10,23` for a fight where Frog has a higher speed than Crono and Robo and gets an additional turn.

The second paramter is the minimum number of crits to search for. 

The third parameter is the number of attacks to consider. If the second and third parameters are `3 6` then a seed will be found that gets at least 3 crits in the first 6 attacks using the threholds provided in the first parameter.

NOTE: If multiple values are provided for the first parameter there must not be any additional spaces between numbers (for example, `10,23,10` not `10, 23, 10`)

#### battle_with_rng

`battle_with_rng` is used to find a battle that has a specific starting RNG value

##### Parameter

The only parameter is hex value of the the specific RNG value to look for.

If multiple RNG values are permitted then a comma delimited list can be supplied.

Example:
`battle_with_rng D,12,25,30,3B,4C,60,6E,81,8E,9C,A8,B8,DA,EB,C,E,10,21,23,1F,1E,27,28,29` would search for a seed that results in any of the given RNG values for this battle.

### log_seed

Executes and prints the results of a given seed.

Each logged action will include the RNG values rolled for that command, as well as additional information such as the RNG value for a battle.

#### Parameters

`-f --filename` The filename of the file to log

`-s --seed` The seed to use when logging

#### Example

Given the following input:
```
# File C:\input\zombor.txt
load
battle
battle
battle_with_crits 0,0,10,0,10 2 5
battle_with_crits 10,10,10,10,10 3 5
```

The following output will be given:

```
> rng_hunter log_seed -s 1699996852 -f "C:\input\zombor.txt"
Loading input file: C:\input\zombor.txt
Done loading
Seed: TimeZones.UTC, 14, 11, 2023, 21, 20, 52 (1699996852)
        load: (4706 3833 3F0C 03D4 3035 66D4 4BE1 0D66 1485 0311 36E1 3B83 321B 52CE 6D6E 48D2 108D 5899 5BEE 53F0 6896 6C8D 3320 5F71 340F 5C76 17B0 1322 7A1A 5736 7009 2291 2BA1 2767 446A 7CD0 23FF 0141 653C 2975 21FC 67BE )
        battle: EF (4BA3)
        battle: 5D (76E5)
        battle crits: 0 in 5 turns from 4D (4CFF)
        battle crits: 1 in 5 turns from 6B (6CFD)
```

When logging, the result of advanced actions will only be logged, no hunting for rng values will be done. In this example the `battle_with_crits` commands don't result in the expected number of crits, but the results are logged.

### find_seeds

Finds seeds that match the requirements from the input file. Once a set of seeds is found each seed will be logged similar to `log_seed`

#### Parameters

`-f --filename` The input file

`-s --start` The Unix time to start looking for seeds

`-e --end` The Unix time to stop looking for seeds

`p --pool` The pool size for threads to use when looking for seeds.

`-m --max_seeds` The maximum number of seeds to find (If the pool size is greater than 1 then this is a best effort value, additional seeds may be found before the additional threads are exited due to the batching done)

#### Example

```
>rng_hunter find_seeds -f "C:\input\zombor.txt" -s 1600000000 -e 1700000000 -m 10 -p 8
Loading input file: C:\input\zombor.txt
Done loading
Finding seeds between 1600000000 and 1700000000
Done! 18 seeds found!
Execution time: 1 ms
Seed: TimeZones.UTC, 13, 09, 2020, 12, 38, 03 (1600000683)
        load: (1702 735F 1BD7 5373 1C84 28D5 67C6 6FF5 7652 7CB2 7A98 7585 6FD3 24D3 2A87 39FF 3DCC 1FFB 641A 7D64 4F81 526B 245F 0B18 3B1D 50B3 6875 7B4E 4953 4CC7 364E 28C0 6446 5955 22B3 2EA3 6DA6 7756 7889 02DA 401C 21E4 )
        battle: B8 (6552)
        battle: 6B (006A)
        battle crits: 2 in 5 turns from 4A (5FE9)
        battle crits: 3 in 5 turns from 98 (4C4B)
Seed: TimeZones.UTC, 13, 09, 2020, 12, 46, 00 (1600001160)
        load: (1D18 2EAD 21D1 782C 2F78 3D6F 0F26 219E 240F 3D1E 6700 4798 12C7 1349 30C5 6BBC 6F42 3B5B 373F 3678 65CD 7FC2 6D38 1943 048F 4E51 0047 6623 0506 2BE3 0DE9 0073 49B5 0B42 2ED3 0AE3 6416 0B47 4C87 36C0 36D6 6042 )
        battle: 9C (7526)
        battle: 8F (1876)
        battle crits: 2 in 5 turns from 4A (0841)
        battle crits: 3 in 5 turns from 09 (6C9B)
Seed: TimeZones.UTC, 05, 02, 2021, 04, 40, 18 (1612500018)
        load: (6AA3 3557 2415 023F 63C2 19A6 3D61 43EA 5555 0373 1DAA 3E3F 3806 7C8E 0C10 2E2D 5E19 32FC 500E 0895 46FC 4E42 41AD 19AB 5120 29E0 29F7 33A0 3E08 191B 07AF 307E 3CA0 17FF 0C3F 4B53 6DCF 0DA5 64F2 1491 174F 03B2 )
        battle: 88 (2463)
        battle: BB (05B5)
        battle crits: 2 in 5 turns from 37 (78BD)
        battle crits: 3 in 5 turns from 98 (7720)
Seed: TimeZones.UTC, 05, 02, 2021, 05, 04, 06 (1612501446)
        load: (7CDA 694B 64AA 566B 23BE 2971 0F05 7B35 1D0C 0CC3 64A0 33F1 1D70 0607 1AB8 7837 3837 2D30 3409 2022 041A 20DD 6C42 71F8 0257 2123 77B6 330D 7C07 478C 6B54 3FE4 49AB 005F 60D9 6F98 3908 2922 2B44 467A 0535 785E )
        battle: BA (2F8A)
        battle: 39 (3BFC)
        battle crits: 2 in 5 turns from 08 (14F2)
        battle crits: 3 in 5 turns from B5 (12A2)
Seed: TimeZones.UTC, 29, 06, 2021, 21, 09, 05 (1625000945)
        load: (5292 10F5 2124 3CC6 3AC2 6DD2 2E89 7BE1 4DCB 30E8 28F1 18E3 4C5B 45BF 051B 4F9C 74CB 5BD9 34CB 09D0 0CA4 5A2D 1C6F 0E45 2363 26D8 0C01 261B 7279 170E 3F2F 2C5C 7E96 1B56 28BA 3BD4 5E27 7BF7 0D1C 6F23 491E 39AD )
        battle: 9E (306D)
        battle: 78 (6710)
        battle crits: 2 in 5 turns from 48 (55F1)
        battle crits: 3 in 5 turns from 09 (66A1)
Seed: TimeZones.UTC, 29, 06, 2021, 21, 58, 07 (1625003887)
        load: (781A 13A8 138C 78E3 43C8 3489 6768 1317 0816 3229 052D 1314 2477 0F3A 4199 2433 5A33 7DFA 63C3 17BC 01C4 5005 25A4 7356 2F80 186D 1DD5 43C2 0B79 340C 769A 07C9 60A2 2C19 1620 4773 2733 26FA 1C03 2B7A 0E4E 5B59 )
        battle: 6C (5813)
        battle: 28 (28FE)
        battle crits: 2 in 5 turns from 2C (48E2)
        battle crits: 3 in 5 turns from B4 (397A)
Seed: TimeZones.UTC, 21, 11, 2021, 13, 14, 09 (1637500449)
        load: (285B 0A8D 3A86 791E 1B3B 7EE4 0AD8 4361 4108 5CFA 6A60 7EB0 2BAC 48A3 20EF 79A4 3D29 7261 594E 3EF5 7425 41DB 71DE 3393 61A5 048A 407C 5AF3 2C19 74B6 4DA7 35A7 4399 2CA1 4AE5 6E32 2B6E 5A02 4862 481A 7CD4 45B4 )
        battle: 4A (6DDB)
        battle: 45 (053F)
        battle crits: 2 in 5 turns from 06 (06FE)
        battle crits: 3 in 5 turns from B4 (03B0)
Seed: TimeZones.UTC, 21, 11, 2021, 13, 33, 13 (1637501593)
        load: (36F3 2A7E 110A 060D 287C 0A77 5F9C 2A1F 7B34 0ACD 556B 4385 20C5 1E08 0429 213B 6011 1419 4E55 64D4 392E 7E4C 0B60 260F 4AAF 0F78 370F 74AA 1C26 4C78 3020 2C7B 71E7 20DF 4A30 4FBC 0E0E 2BE8 36DC 5BA7 02C3 59AC )
        battle: 32 (0A27)
        battle: 8F (4B43)
        battle crits: 2 in 5 turns from 37 (79BC)
        battle crits: 3 in 5 turns from B5 (1C98)
Seed: TimeZones.UTC, 21, 11, 2021, 13, 51, 02 (1637502662)
        load: (4496 7D7C 75EA 092F 67E4 17A1 2486 6A76 4FFE 41D0 6BCB 7B73 3FDF 03DE 01C7 714F 5224 1FCD 2B04 1EB5 6DD5 034E 7602 117B 7D83 7284 4AE4 3385 1C93 500B 236A 72E6 2ED9 2639 7F1F 3549 1669 55A4 67D4 1A90 7BA9 0CE4 )
        battle: 67 (1A4C)
        battle: 0E (52BA)
        battle crits: 2 in 5 turns from 08 (5BAB)/
        battle crits: 3 in 5 turns from 09 (6D9A)
Seed: TimeZones.UTC, 21, 11, 2021, 13, 56, 07 (1637502967)
        load: (487A 0334 1966 065E 68BE 5DF6 20B8 4AE3 2801 24F7 3B94 2FED 4845 0544 49A8 2204 2144 5FBB 3026 1A25 0E56 0F64 56C4 36B7 7396 6A05 7608 60C0 1E40 2EF0 1AC2 5159 045E 5820 02DA 0B60 27A4 0184 374B 1D6C 1F98 5A9B )
        battle: 78 (0770)
        battle: 47 (4006)
        battle crits: 2 in 5 turns from 06 (0EF6)
        battle crits: 3 in 5 turns from 98 (5641)
```

### extend_seed

Finds the number of extra RNG rolls needed for the final command in the input file to succeed for the given seed. The output is broken down by the number of heals (1 RNG roll each) and number of extra room transitions (66 RNG rolls, extra room transitions are in sets of 2, each transition takes 33 RNG rolls, so 66 rolls for an extra set of room transitions) needed.

#### Parameters

`-s --seed` The seed to extend

`-f --filename` The input file

`m --max_rolls` The maximum number of additional RNG rolls to consider when extending the seed

#### Example

```
>rng_hunter extend_seed -f "C:\input\zombor.txt" -s 1675002659
Loading input file: C:\input\zombor.txt
Done loading
        load: (58E1 776C 0C38 003C 41E7 44F3 4018 6C7A 4DD6 766C 561D 2C5C 0498 7A27 3124 4DDE 588E 6416 1429 4B9C 4F2F 03DE 300A 738A 1CF2 212E 3E8D 25E5 386E 5250 4EA5 006A 28C0 20FE 25F1 49C5 132E 7176 5B86 314C 4C0A 5FD1 )
        battle: FE (0FEE)
        battle: 25 (36ED)
        battle crits: 2 in 5 turns from 48 (6ED8)
Rolls: 1, (0 rooms, 0 heals)
Rolls: 3, (0 rooms, 2 heals)
Rolls: 17, (0 rooms, 16 heals)
Rolls: 23, (0 rooms, 22 heals)
Rolls: 170, (4 rooms, 37 heals)
Rolls: 243, (6 rooms, 44 heals)
Rolls: 348, (10 rooms, 17 heals)
Rolls: 493, (14 rooms, 30 heals)
Rolls: 531, (16 rooms, 2 heals)
Rolls: 598, (18 rooms, 3 heals)
Rolls: 644, (18 rooms, 49 heals)
```