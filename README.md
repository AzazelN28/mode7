# Mode 7

Mode 7 implementation in C for WebAssembly.

[Demo](https://azazeln28.neocities.org/games/mode7)
[Demo (pure JS)](https://codepen.io/AzazelN28/pen/yZyJbe)

## How to compile this?

### Emscripten

First of all you need to install emscripten and its `emcc` compiler. Follow the installation guide that you will find in [Emscripten](https://kripken.github.io/emscripten-site/index.html).

### Compiling

Once you have `emcc` properly installed run this to generate a debuggable `mode7.js` and `mode7.wasm`:

```sh
build.dev.sh
```

## How to use this library?

Its easy, you only need to import `mode7.js` in your webpage and then:

```html
<script src="mode7.js"></script>
<script>

  Module.onRuntimeInitialized = async _ => {
    // Allocate 64x64x4 to draw the plane texture in it.
    const PlaneSize = 64
    const planePointer = Module._get_safe_memory(PlaneSize * PlaneSize * 4)
    const plane = new ImageData(new Uint8ClampedArray(Module.buffer, planePointer, PlaneSize * PlaneSize * 4), PlaneSize, PlaneSize)

    // Allocate 320x200x4 to draw the final image.
    const targetPointer = Module._get_safe_memory(320 * 200 * 4)
    const target = new ImageData(new Uint8ClampedArray(Module.buffer, targetPointer, 320 * 200 * 4), 320, 200)

    // Initialize mode 7.
    Module._set_perspective(Math.PI * 0.5, 0.001, 0.006)
    Module._set_plane(plane.width, plane.height, planePointer)
    Module._set_view(angle, x, y)

    let frameID
      , angle = 0
      , x = 0
      , y = 0

    function frame(time) {
      if (keys.get('ArrowLeft')) {
        angle -= 0.05
      } else if (keys.get('ArrowRight')) {
        angle += 0.05
      }

      if (keys.get('ArrowUp')) {
        x += Math.cos(angle) * 0.001
        y += Math.sin(angle) * 0.001
      } else if (keys.get('ArrowDown')) {
        x -= Math.cos(angle) * 0.001
        y -= Math.sin(angle) * 0.001
      }

      // Updates the view angle and position.
      Module._set_view(angle, x, y)
      // Renders the pseudo-plane.
      Module._render(target.width, target.height, targetPointer)

      cx.putImageData(target, 0, 0)

      frameID = window.requestAnimationFrame(frame)
    }

    frameID = window.requestAnimationFrame(frame)
  }

</script>
```

## Thanks to

- [javidx9](https://twitter.com/javidx9) for his great video explanation [Programming Pseudo 3D Planes aka MODE7 (C++)](https://www.youtube.com/watch?v=ybLZyY655iY)
- All the people working on Emscripten.
- [Lin Clark](https://twitter.com/linclark) for her articles on [WebAssembly Memory](https://hacks.mozilla.org/2017/07/memory-in-webassembly-and-why-its-safer-than-you-think/)

Made with :heart: by [AzazelN28](https://github.com/AzazelN28)
