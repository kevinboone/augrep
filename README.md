# augrep: search for text in metadata of audio files 

`augrep` is simple utility for searchiing for regular expressions in the
metadata ('tags') of a range of audio files. Presently the utility supports
ID3v2 tags, Vorbis comments, and QuickTime 'meta' atoms; these tags are
found in MP3, Ogg, FLAC, AAC, M4A, and M4B files, among others. 
`augrep` pays no attention to file extension -- all files are searched
for all recognized tagging schemes, regardless of name. 

By default the utility displays the matching file, and any metadata
entries where the match occurred. It can search directories recursively
if required. 

## Example

```
$ augrep -ir\ '\bmozart\b' audio 
```

Find files in the `audio` directory and its subdirectories that contain
the whole word 'mozart', independent of case, in any tag.
 

## Match pattern

The pattern to match is a regular expression, optionally preceded by a
tag label and an equals sign:

```
label=regex
```

If no label is given, then all metadata elements are searched. 

The list of search labels that _should_ be common across tag
formats is: album, artist, album-artist, comment, composer, date, 
genre, title, track. It is possible to search for a format-specific
tag label as well but, of course, that will only match files of a particular
type.

## Build and install 

`augrep` is written in C, but uses GNU-specific extensions to the 
standard C library. It should be possible to build it on any 
system that has a `gcc` compiler; on other systems YMMV.
In general:

```
$ make
$ sudo make install
```

## Further information 

After installation `man augrep` should give you full details of the
various command-line options. 

 
## Author and legal

`augrep`
is maintained by Kevin Boone, and is open source under the
terms of the GNU Public Licence, version 3.0. There is no warranty
of any kind.

