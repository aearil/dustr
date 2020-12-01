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

## TODO ##

* Add magnifying glass feature
* Add keyboard movement
* Batch processing
* stdin/stdout IO
* Implicit output filename
