# MMDPreviewGenerator

---

Usage: `MMDPreviewGenerator.exe width height modelPath previewImageStorePath`

Examples: `MMDPreviewGenerator.exe 1920 1440 "C:\Temp\ポプ子\ポプ子.pmx" "C:\Temp\ポプ子\ポプ子_preview.png"`

---

### Restrictions: 

  * For now No Support for glReadPixels when obscured pixels
  
  * Output png only

### Suggestions:

   * Resolution parameters should below (not equal, depends on your taskbar size) your monitor resolution

   * Or, use it with VM to create virtual monitor when huge resolutions


### Shaders

  * currently only albedo(vec4) * diffuse(vec4)
  
  * Since no toon tinted & enlighten from enviornment map, the color may less correct
  
  * Besides, Alpha may not handles very well

  * However for now it is sufficient for **"Preview"** purpose.

  * Our Results:

    ![](https://www.imgur.com/UEti756.png)
    
    ![](https://www.imgur.com/DmCd5JZ.png)

  * Results From MMD:

    ![](https://www.imgur.com/UVAz8GN.jpg)
    
    ![](https://www.imgur.com/WlhyJtw.jpg)

