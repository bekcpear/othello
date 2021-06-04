#!/bin/sh

cd "$(dirname $0)"/../../po/

wxrc -g ../wx/resources/frame.xrc > ../wx/resources/frame.xrc.msgs.c

intltool-update -p
intltool-update $(cat LINGUAS)

