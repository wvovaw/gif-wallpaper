<h1 align="center">
    gif-wallpaper
</h1>

> Set gifs as wallpaper on Windows

~~Paperview~~ Gif-wallpaper is a high performance animated desktop background setter for ~~Linux and X11~~ OS Windows.
The original program is [Paperview](https://github.com/glouw/paperview).
Gif-wallpaper uses its codebase with x11 to winapi replacements.
Gif-wallpaper itself based on  TrAyZeN's version of [sdl-wallpaper](https://github.com/TrAyZeN/sdl-wallpaper).

## Requirements
- [CMake](https://cmake.org/download/)
- [SDL2](https://www.libsdl.org/download-2.0.php)

## Build
> First of all download [SDL2.dll](https://www.libsdl.org/release/SDL2-2.0.14-win32-x64.zip)
And [SDL2-devel](https://www.libsdl.org/release/SDL2-devel-2.0.14-VC.zip)
> Then clone the project
```
git clone https://github.com/wvovaw/gif-wallpaper.git
cd gif-wallpaper
```
> After move *SDL2.dll* and devel dir to the project dir
> Cmake needs to know which Visual Studio you're gonna use. Run the command bellow and choose it from the output
```
cmake -G
```
> And run the command
```
cmake.exe -G "Visual Studio 16 2019" . -DSDL2_INCLUDE_DIR:FILEPATH=.\SDL2-2.0.14\include -DSDL2_LIBRARY:FILEPATH=.\SDL2-2.0.14\lib
```
This should generate solution file, just open it with **Visual Studio X** 
> To build and debug project follow this steps:
- Run *gif-wallpaper.sln*;
- Set gif-wallpaper project as startup;
- Add comandline arguments in **project properties/debugging**: *./assets/moomoo 100*;
- Build the project and run!
> Done! Now you can debug the program

## Or download the latest build from Releases

## Usage
### Single Monitor Use
```
./paperview.exe FOLDER SPEED
```
*SPEED is the delay time in miliseconds between two frames rendering.
FOLDER is where all **frame-x.bmp** placed. Only BMP files are supported.*

### Creating Custom Scenes

Creating a custom BMP scene folder from a GIF requires the [imagemagick](https://imagemagick.org/script/download.php#windows).
Install it and don't forget to check the box *Install legacy utils*, 'cause it includes *convert* util.
Example, to create a castle scene folder from a castle.gif:

```
mkdir castle
mv castle.gif castle
cd castle
convert -coalesce castle.gif out.bmp
rm castle.gif
```

## Demo

![moomoo.gif](https://s2.gifyu.com/images/moomoo.gif)
