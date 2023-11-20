<br/>
<p align="center">
  <h3 align="center">ROAMSimple SDL</h3>

  <p align="center">
    Port of Bryan Turner's ROAM Real-Time Dynamic Level of Detail Terrain Rendering demo.
    <br/>
    <br/>
  </p>
</p>



## About The Project

This is a port of Bryan Turner's Real-Time Dynamic Level of Detail Terrain Rendering with ROAM demo.

The original article can be found on https://www.gamedeveloper.com/programming/real-time-dynamic-level-of-detail-terrain-rendering-with-roam, and a version that I made is available as article.pdf, on the root of the repository.

The original code was written using GLUT, for Win9X platforms, in the early 2000s, and will not run on newer platforms. It was not available on the above link anymore, but I could download it using archive.org. It is available in the _original_src folder - nothing was changed.

My port basically replace GLUT with SDL2 to manage the window, events and so on, but retains most of the ROAM code as close as the original as possible. There are some codestyle changes, and a little refactor, but nothing major.

## Authors

* **Bryan Turner** - *Developer* - [Bryan Turner](https://www.gamedeveloper.com/programming/real-time-dynamic-level-of-detail-terrain-rendering-with-roam) - *Original Project*
* **Rodrigo Verdiani** - *Developer* - [Rodrigo Verdiani](www.roverdi.dev) - *Port to SDL*
