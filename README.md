# Alien Lander

I’ve been playing around trying to replicate the amazing computer graphics [Alan
Sutcliffe](http://www.bcs.org/content/conWebDoc/52263) created for the [landing
sequence](https://www.flickr.com/photos/mr_carl/6047866844/in/set-72157627318864775/)
in the 1979 movie Alien. Mr Sutcliffe wrote [an amazing article about his "Magic
Mountains"](https://archive.org/stream/creativecomputing-1981-06/Creative_Computing_v07_n06_1981_June?ui=embed#page/n53/mode/2up)
project in the June 1981 issue of Creative Computing that explains his technique
in detail.

![Screenshot](PREVIEW.png)

I'm taking a much lazier approach and just throwing modern CPU at it. My version
uses Cinder 0.9.x and requires OpenGL 3.2.

Right now it just lets you fly across a low resolution [height map of the United
States](http://bit-player.org/2009/long-division).

Credits:

- [16-segment font](http://www.msarnoff.org/alpha32/)
- [Map of US](http://bit-player.org/2009/long-division)
