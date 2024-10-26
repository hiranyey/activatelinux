# Activate linux

Build
- make build

Install
- make install

Run
- ./main or activatelinux "FontName-FontSize"
  E.g:-
  activatelinux "Ubuntu Sans Mono-14"
  ./main "Ubuntu Mono-12"

in i3 Config add

```
exec --no-startup-id activatelinux "Ubuntu Mono-12"
for_window [title="ActivateLinux"] floating enable sticky enable
```

in picom or your compositor,disable all the blur,shadow,animation for this window

```
{
 match = "name='ActivateLinux'";
shadow =false;
fading =false;
blur-background =false;
}
```

References

- https://ftp.dim13.org/pub/doc/Xlib.pdf
- https://www.youtube.com/watch?v=764fnfEb1_c
