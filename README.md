# Dustr #

The light and interactive tool your crops need!

Dustr is a interactive and visual cropping tool, because even though the CLI is the best thing since sliced bread, cropping is one of those operation where seeing what you are doing is really more effective.

This tool is especially useful when you know the output size, but you need to pick the most aesthetically pleasing crop of the image.

## Usage ##

Click to crop at a given geometry:
```
dustr -g 200x300 in.png out.jpg
```

Click and drag to crop:
```
dustr -s in.jpg selection.png
```

## Tips ##

Fit an image to a specific format while keeping the aspect ratio. Useful for preprocessing huge wallpapers.

```
convert huge.jpg -resize "1920x1080^" resized.jpg
```

You can use this command to directly set a background from an ill-sized wallpaper
```
convert wallpaper.jpg -resize "1920x1080^" - | dustr -g 1920x1080 - - | feh --bg-fill -
```

## Installation ##

Simple build to try it out:
```
make
```

System wide install:
```
sudo make install
```

## TODO ##

* Add magnifying glass feature
* Add keyboard movement
* Batch processing
* Implicit output filename
