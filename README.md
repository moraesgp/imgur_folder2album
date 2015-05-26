# imgur_folder2album
Imgur client that uploads images to imgur transforming directories (folders) in albums

## Depends on:

  * Curl 7.35.0 or higher
  * FreeImage 3.15.4 or higher

On Ubuntu just run

    sudo apt-get install \
    	libcurl3 libcurl3-dbg libcurl3-gnutls libcurl4-gnutls-dev \
    	libfreeimage3 libfreeimage-dev libfreeimage3-dbg

## Build

just run

    make

to develope use 

    make runme
	
It will create an executable binary called runme that contains gdb symbols to debug

On Windows using Visual Studio C compilers run the following to generate a debug version

    nmake /NOLOGO /F makefile.nmake

Or the following to generate a production version

	nmake /NOLOGO /F makefile.nmake DBGCFLAGS= DBGLINKERFLAGS=

## Uploading images

to upload images, organize them in a folder structure. The structure below will be used as an example.

    /tmp/somedir/image_root_dir
                       .
                       |-- P1050250.JPG
                       |-- P1050257.JPG
                       |-- another_dir_name
                       |   `-- 2012_07
                       |       |-- IMG_20120708_152103.jpg
                       |       |-- IMG_20120708_160547.jpg
                       |       `-- IMG_20120708_175629.jpg
                       |-- some_dir
                       |   `-- 2012_07
                       |       |-- P1050222.JPG
                       |       |-- P1050225.JPG
                       |       |-- P1050226.JPG
                       |       |-- P1050244.JPG
                       |       |-- P1050246.JPG
                       |       |-- P1050247.JPG
                       |       |-- P1050248.JPG
                       |       |-- P1050249.JPG
                       |       |-- P1050255.JPG
                       |       |-- P1050256.JPG
                       |       |-- P1050258.JPG
                       |       |-- P1050260.JPG
                       |       |-- P1050262.JPG
                       |       |-- P1050263.JPG
                       |       |-- P1050268.JPG
                       |       |-- P1050274.JPG
                       |       |-- P1050276.JPG
                       |       `-- P1050277.JPG
                       `-- whatever
                           `-- 2012_06
                               |-- photo_0000000001.jpg
                               |-- photo_0000000002.jpg
                               |-- photo_0000000003.jpg
                               |-- photo_0000000004.jpg
                               |-- photo_0000000005.jpg
                               |-- photo_0000000006.jpg
                               |-- photo_0000000008.jpg
                               |-- photo_0000000009.jpg
                               |-- photo_0000000010.jpg
                               |-- photo_0000000011.jpg
                               `-- photo_0000000012.jpg

directories *another_dir_name*, *some_dir* and *whatever* will be ignored.

Two albums will be created: *2012_07* and *2012_06*. Note there are two directories named 2012_07. Their pictures will be combined into only one album 2012_07.

Picture *P1050250.JPG* and *P1050257.JPG* will be uploaded but won't be added to any album because they are in the root directory


