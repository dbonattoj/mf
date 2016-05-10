#!/bin/sh
grep -rn '// *TODO' src/ | sed 's|^\([^:]*:[^:]*:\).*//[[:space:]]*TODO|\1|'
