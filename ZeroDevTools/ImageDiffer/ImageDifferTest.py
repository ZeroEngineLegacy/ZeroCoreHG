import getopt
import sys
import argparse
from os import listdir
from os.path import isfile, join
from shutil import copy2
from subprocess import call

def test(originalDirectory, destinationDirectory, confidence):
  tests = [ f for f in listdir(originalDirectory) if isfile(join(originalDirectory,f)) ]

  for test in tests:
    # Copy original file to destinationDirectory.
    copy2(join(originalDirectory, test), join(destinationDirectory, 'Original-' + test))

    # Diff the images.
    command=['ImageDiffer.exe', 
             '-file1',   join(destinationDirectory, 'Original-' + test),
             '-file2',   join(destinationDirectory, test),
             '-output1', join(destinationDirectory, 'Diff-' + test),
             '-output2', join(destinationDirectory, 'DiffSq-' + test),
             '-output3', join(destinationDirectory, 'DiffBW-' + test),
             '-confidence', confidence ]

    call(command)


def main (argv):
  parser = argparse.ArgumentParser(description='Diff the original images in the original '
                                               'directory, with the images in the '
                                               'destination directory.')
  parser.add_argument('original', action='store',
                      help='Original directory where the original files are.')
  parser.add_argument('destination', action='store',
                      help='Destination directory to put the diffed '
                           'files and where to copy the originals to.')
  parser.add_argument('confidence', action='store',
                      help='Float for determining how accurate our '
                           'image should be before emitting a warning.')

  args = parser.parse_args()
  test (args.original, args.destination, args.confidence)


if __name__ == "__main__":
  main(sys.argv[1:])
