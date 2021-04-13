To enable source index open visual studio and go to Tools->Options. Look under Debugging->General.
There should be the option "Enable source server support", turn that on.
You should probably also turn on the sub-option to print diagnostic messages.

To enable the source server, go to Tools->Options. Look under Debugging->Symbols.
Add the source server's location in this dialog. For me it's \\ZEROBUILDWINDOW\SymbolStore\Symbols.
The pdb's from the symbol server will look for a zero repro under the environmental varaible %ZERO_SOURCE%.
Either define this system wide or set it locally before opening the .dmp. This depo must be up-to-date when
trying to check a dump otherwise it'll fail to find the file (the depo needs to contain the version the dump is looking for).

To not have to not have to deal with the trusted command that pops up when it
tries to run the mercurial command, copy srcsrv\srcsrv.ini to your visual studio directory's
common7\ide folder. (On this machine that's "C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE")