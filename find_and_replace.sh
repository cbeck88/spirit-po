#!/bin/bash
find include test test_libintl -type f -print0 | xargs -0 sed -i "s|(C) Copyright Christopher Beck|(C) Copyright 2015 - 2016 Christopher Beck|g"
