#!/usr/bin/env sh

set -e +x

echo "Creating zip file to submit..."

cd /home/exame/pp2/
zip -r pp2.zip Makefile conf/* *.c *.h

echo ""
echo "You can now submit your ZIP file."

