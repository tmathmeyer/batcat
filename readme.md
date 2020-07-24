# batcat

write your battery power to a csv file (linux)


## building
run ```make```

## running
```batcat -b [battery device] -f [output_file] -d```
where
  - [battery device] is something like BAT0 or BAT1 (check
`/sys/class/power_supply/`)
  - [output file] is any path to a file
  - (-d) puts it in debug mode (aka, prints the csv to console)
