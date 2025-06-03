rmdir /s build
python build.py
robocopy /z . D: "MICROBIT.hex"