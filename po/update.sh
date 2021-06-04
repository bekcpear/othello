#!/bin/sh

cd $(dirname $0)

wxrc -g ../wx/resources/frame.xrc > ../wx/resources/frame.xrc.msgs.c
intltool-update $(cat LINGUAS)

