c     $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/testpf.f,v 1.1 91/12/06 17:27:50 harrison Exp Locker: harrison $
      character*60 fname
      call pbeginf
      fname = ' '
      write(fname,'(a,i3.3)') '/tmp/pfcopy.test',nodeid()
      call pfcopy(5, 0, fname)
      call pend
      end
