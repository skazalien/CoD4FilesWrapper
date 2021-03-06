# *Cod4FilesWrapper*

This **.exe** file copies all map-related files to your CoD4 Directory **'z_wrapped_maps'** folder, so that it can be quickly pushed to your own Map Repository, or just to have a quick backup in general that you can upload anywhere you'd like. You can also share the map easily with someone else, without going through the process of manually selecting and copying all individual files and compiling .gdt's in AssetManager.

This tool is mostly made for ***CodJumper***, so if you happen to notice any filetype that isn't wrapped, please let me know on my discord, so that I can look into it:
***skazy#2211***

The tool is meant to be really simplified, not *GUI* dependant.

## Includes
- *.setting*
- *.gdt*
- *.efx*
- *.csv*
- *.iwi*
- *.gsc*
- *.sun*
- *prefab*
- *images*
- *materials*
- *materials_properties*
- *xmodel files*
- *playerModel files*

Copying all .iwi files from **'z_wrapped_maps/***YourMapName***/raw/images'** can be put immediately into your 'map.iwd'.
The wrapped map folder contains the optional ___YourMapName___**_OPTIONAL** subfolder containing all Image files. These are not required to have a functioning copy of your map.

## How To Use

Just place it anywhere you'd like, with all the files it comes with[^1], preferably in it's own folder.
Run the .exe, and type in your main .map file Path. If this .map file isn't situated in your CoD4 Directory 'map_source' subfolder, you'll have to define your own CoD4 Directory as well.

And thats it, your map copy is done!

### BONUS:

.exe itself may seem a bit slow, as it goes through every .map file your main Map has, looking for prefabs, models, their corresponding textures, etc.


### TODO:
- [ ] Adding more Optional Files[^2]

[^1]: stockFiles.txt
[^2]: These are files that'd most likely would overwrite other's particular file(s).
