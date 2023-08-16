      program scf
      implicit double precision (a-h, o-z)
      include 'cscf.h'
      include 'msgtypesf.h'
      dimension orbs(nbfn*nbfn), dens(nnbfn), fock(nnbfn),
     $     work(nbfn*nbfn), evals(nbfn)
      data tinit, tonel, ttwoel, tdiag, tdens, tprint /6*0.0d0/
      data eone, etwo, energy, deltad /4*0.0d0/
c     
c     initalize the parallel message passing environment
c     
      call pbeginf
      me = nodeid()
      nproc = nnodes()
c     
c     initialize a bunch of stuff and zero initial density matrix
c     
      rjunk = timer()
      call ininrm
      call dfill(nnbfn, 0.0d0, dens, 1)
      tinit = timer()
c
c     make initial orthogonal orbital set for jacobi diagonalizer
c
      call makeob(orbs, work)
c     
c     iterate
c     
      do 10 iter = 1, mxiter
c     
c     make info for sparsity test ... redone every iter to save space
c     
         call makesz(work, schwmax)
c     
c     make the one particle contribution to the fock matrix (in fock)
c     and the partial contribution to the energy
c     
         call oneel(dens, work, schwmax, fock, eone)
         call dgop(1, eone, 1, '+')
         tonel = tonel + timer()
c     
c     compute the two particle contribution and then add up the
c     contributions from each process with dgop
c     
         call twoel(dens, work, schwmax, fock, etwo)
         call dgop(2, etwo, 1, '+')
         call dgop(3, fock, nnbfn, '+')
         ttwoel = ttwoel + timer()
c     
c     only process 0 diagonalizes and updates the density
c     
         if (me.eq.0) then
c     
c     diagonalize the fock matrix ...
c     
            call diagon(fock, orbs, evals, work, tester)
            tdiag = tdiag + timer()
c     
c     make the new density matrix in work from orbitals in orbs,
c     compute the norm of the change in the density matrix and
c     then update the density matrix in dens with damping.
c     
            call makden(orbs, work)
            deltad = dendif(dens, work)
            scale = 0.0d0
            if (iter.gt.1) then
               if (deltad.gt.0.5d0) then
                  scale = 0.5d0
               else if (deltad.gt.0.05d0) then
                  scale = 0.25d0
               else 
                  scale = 0.0d0
               endif
            endif
            call damp(scale, dens, work)
            tdens = tdens + timer()
c     
c     add up energy and print out convergence information
c     
            energy = enrep + eone + etwo
            call prnout(iter, energy, deltad, tester)
            tprint = tprint + timer()
         endif
c     
c     brodcast new density matrix and deltad to everyone
c     
         call brdcst(4+MSGDBL, dens, mdtob(nnbfn), 0)
         call brdcst(5+MSGDBL, deltad, mdtob(1), 0)
c     
c     if converged then exit iteration loop
c     
         if (deltad .lt. tol) goto 20
 10   continue
      if(me.eq.0)
     $     write(6,*) ' SCF failed to converge in ', mxiter, ' iters'
c     
c     finished ... print out eigenvalues and occupied orbitals
c     
 20   continue
      call igop(6, icut1, 1, '+')
      call igop(7, icut2, 1, '+')
      call igop(8, icut3, 1, '+')
      if (me.eq.0) then
c     
c     print out timing information
c     
         call prnfin(energy, evals, orbs)
         write(6,1) tinit, tonel, ttwoel, tdiag, tdens, tprint,
     $        nproc
 1       format(/'   init   onel  twoel   diag   dens   print  ncpu'/
     $        '  ------ ------ ------ ------ ------ ------ ------'/
     $        1x, 6f7.2, i7/)
c     
c     print out information on # integrals evaulated each iteration
c     
         nints = nnbfn*(nnbfn+1)/2
         frac  = dfloat(icut3)/dfloat(nints)
         write(6,2) icut1, icut2, icut3, nints, frac
 2       format(/'       No. of integrals screened or computed '
     $        /'       -------------------------------------'/
     $        /1x,'   #ij test   #kl test   #compute     #total',
     $        '  fraction',
     $        /1x,'  ---------  ---------  ---------  ---------',
     $        '  --------',
     $        /1x,4(2x,i9),f9.3)
      endif
c     
      call pend
      call fexit
c     
      end
      subroutine makesz(schwarz, schwmax)
      implicit double precision (a-h, o-z)
      include 'cscf.h'
      include 'msgtypesf.h'
      dimension schwarz(nnbfn)
c
c     schwarz(ij) = (ij|ij) for sparsity test
c
      icut1 = 0
      icut1 = 0
      icut3 = 0
      ij = 0
      schwmax = 0.0d0
      call dfill(nnbfn, 0.0d0, schwarz, 1)
c
      me = nodeid()
      nproc = nnodes()
      do 10 i = 1, nbfn
         do 20 j = 1, i
            ij = ij + 1
            if (mod(ij,nproc).eq.me) then
               call g(gg, i, j, i, j)
               schwarz(ij) = sqrt(gg)
               schwmax = max(schwmax, schwarz(ij))
            endif
 20      continue
 10   continue
c
      call dgop(101+MSGDBL, schwarz, nnbfn, '+')
      call dgop(102+MSGDBL, schwmax, 1, 'max')
c
      end
      subroutine ininrm
      implicit double precision (a-h, o-z)
      include 'cscf.h'
c
c     write a little welcome message
c
      if (nodeid().eq.0) write(6,1) natom, nocc, nbfn, tol
1     format(/' Example Direct Self Consistent Field Program '/
     $        ' -------------------------------------------- '//
     $        ' no. of atoms ............... ',i3/
     $        ' no. of occupied orbitals ... ',i3/
     $        ' no. of basis functions ..... ',i3/
     $        ' convergence threshold ...... ',d9.2//)
c
c     generate normalisation coefficients for the basis functions
c     and the index array iky
c
      do 10 i = 1, nbfn
         iky(i) = i*(i-1)/2
 10   continue
c
      do 20 i = 1, nbfn
         rnorm(i) = (expnt(i)*2.0d0/pi)**0.75d0
 20   continue
c
c     initialize common for computing f0
c
      call setfm
c
      end
      double precision function h(i,j)
      implicit double precision (a-h, o-z)
      include 'cscf.h'
cvd$r novector
cvd$r noconcur
c
c     generate the one particle hamiltonian matrix element
c     over the normalized primitive 1s functions i and j
c
      f0val = 0.0d0
      sum = 0.0d0
      rab2 = (x(i)-x(j))**2 + (y(i)-y(j))**2 + (z(i)-z(j))**2
      facij = expnt(i)*expnt(j)/(expnt(i)+expnt(j))
      expij = exprjh(-facij*rab2)
      repij = (2.0d0*pi/(expnt(i)+expnt(j))) * expij
c
c     first do the nuclear attraction integrals
c
      do 10 iat = 1, natom
         xp = (x(i)*expnt(i) + x(j)*expnt(j))/(expnt(i)+expnt(j))
         yp = (y(i)*expnt(i) + y(j)*expnt(j))/(expnt(i)+expnt(j))
         zp = (z(i)*expnt(i) + z(j)*expnt(j))/(expnt(i)+expnt(j))
         rpc2 = (xp-ax(iat))**2 + (yp-ay(iat))**2 + (zp-az(iat))**2
c
         call f0(f0val, (expnt(i)+expnt(j))*rpc2)
         sum = sum - repij * q(iat) * f0val
 10   continue
c
c     add on the kinetic energy term
c
      sum = sum + facij*(3.0d0-2.0d0*facij*rab2) *
     $     (pi/(expnt(i)+expnt(j)))**1.5d0 * expij
c
c     finally multiply by the normalization constants
c
      h = sum * rnorm(i) * rnorm(j)
c
      end
      double precision function s(i,j)
      implicit double precision (a-h, o-z)
      include 'cscf.h'
c
c     generate the overlap matrix element between the normalized
c     primitve gaussian 1s functions i and j
c
      rab2 = (x(i)-x(j))**2 + (y(i)-y(j))**2 + (z(i)-z(j))**2
      facij = expnt(i)*expnt(j)/(expnt(i)+expnt(j))
      s = (pi/(expnt(i)+expnt(j)))**1.5d0 * exprjh(-facij*rab2) *
     $     rnorm(i)*rnorm(j)
c
      end
      subroutine makden(orbs, dens)
      implicit double precision (a-h, o-z)
      include 'cscf.h'
      dimension orbs(nbfn, nbfn), dens(nnbfn)
c
c     generate density matrix from orbitals in orbs. the first
c     nocc orbitals are doubly occupied. Note that the diagonal
c     elements are scaled by 0.5
c
      ij = 0
      do 10 i = 1, nbfn
         do 20 j = 1, i
            p = 0.0d0
            do 30 k = 1, nocc
               p = p + orbs(i,k)*orbs(j,k)
 30         continue
            ij = ij + 1
            dens(ij) = 2.0d0 * p
 20      continue
         dens(ij) = dens(ij)*0.5d0
 10   continue
c
      end
      subroutine oneel(dens, schwarz, schwmax, fock, eone)
      implicit double precision (a-h, o-z)
      include 'cscf.h'
      dimension dens(nnbfn), fock(nnbfn), schwarz(nnbfn)
c
c     fill in the one-electron part of the fock matrix and
c     compute the one-electron energy contribution
c
c     simple structure to share out the work between processes
c
      me = nodeid()
      nproc = nnodes()
c
      call dfill(nnbfn, 0.0d0, fock, 1)
      do 10 i = me+1, nbfn, nproc
         do 20 j = 1,i
            ij = iky(i) + j
            if (schwarz(ij)*schwmax.gt.tol2e) fock(ij) = h(i,j)
 20      continue
 10   continue
      eone = ddot(nnbfn, fock, 1, dens, 1)
c
      end
      function nxtask(nproc)
      parameter (ichunk = 10)
      save icount,nleft
      data nleft,icount /0,0/
c
c     wrapper round nxtval() to increase granularity
c     and thus reduce no. of requests to shared counter
c
      if(nproc.gt.0) then
         if(nleft.eq.0) then
            icount = nxtval(nproc) * ichunk
            nleft = ichunk
         endif
         nxtask = icount
         icount = icount + 1
         nleft = nleft -1
      else
          nleft = 0
          nxtask = 0
          junk = nxtval(nproc)
      endif
      end
      subroutine twoel(dens, schwarz, schwmax, fock, etwo)
      implicit double precision (a-h, o-z)
      include 'cscf.h'
      dimension dens(nnbfn), fock(nnbfn), schwarz(nnbfn)
c     
c     add in the two-electron contribution to the fock matrix
c     
      nproc = nnodes()
c     
      gg = 0.0d0
c     
      next = nxtask(nproc)
      do 10 i = nbfn, 1, -1
         do 20 j = i, 1, -1
            ij = iky(i) + j - 1
            if (ij .eq. next) then
               if (schwarz(ij)*schwmax .lt. tol2e) then
                  icut1 = icut1 + ij
               else
                  do 30 k = 1, i
                     lhi = k
                     if (k.eq.i) lhi = j
                     do 40 l = 1, lhi
                        kl = iky(k) + l
                        if (schwarz(ij)*schwarz(kl).lt.tol2e) then
                           icut2 = icut2 + 1
                        else
                           icut3 = icut3 + 1
c     
c     compute value of integral (ij|kl) and add into fock matrix
c     
                           call g(gg, i, j, k, l)
                           call addin(gg*0.5d0, i, j, k, l, fock,
     $                          dens, iky)
                        endif
 40                  continue
 30               continue
               endif
               next = nxtask(nproc)
            endif
 20      continue
 10   continue
c     
      etwo = ddot(nnbfn, fock, 1, dens, 1)
c     
c     wait for all processes to finish work
c     
     j     unk = nxtask(-nproc)
c     
      end
      subroutine damp(fac, dens, work)
      implicit double precision (a-h, o-z)
      include 'cscf.h'
      dimension dens(nnbfn), work(nnbfn)
c
      ofac = 1.0d0 - fac
      do 10 i = 1, nnbfn
         dens(i) = fac*dens(i) + ofac*work(i)
10    continue
c
      end
      subroutine prnout(iter, energy, deltad, tester)
      implicit double precision (a-h, o-z)
      include 'cscf.h'
c
c     printout results of each iteration
c
      write(6,1) iter, energy, deltad, tester
1     format(' iter=',i3,', energy=',f13.8,', deltad=',d9.2,
     $     ', deltaf=',d9.2)
c
      end
      double precision function dendif(dens, work)
      implicit double precision (a-h, o-z)
      include 'cscf.h'
      dimension dens(nnbfn), work(nnbfn)
c
c     compute largest change in density matrix elements
c
      denmax = 0.0d0
      do 10 i = 1, nnbfn
         denmax = max(denmax, abs(work(i)-dens(i)))
10    continue
      dendif = denmax
c
      end
      subroutine prnfin(energy, evals, orbs)
      implicit double precision (a-h, o-z)
      include 'cscf.h'
      dimension evals(nbfn), orbs(nbfn, nbfn)
c
c     printout final results
c
      write(6,1) energy
 1    format(//' final energy = ',f16.11//' eigenvalues')
      call output(evals, 1, nbfn, 1, 1, nbfn, 1, 1)
      write(6,2)
2     format(//' eigenvectors ')
      call output(orbs, 1, nbfn, 1, nocc, nbfn, nbfn, 1)
c
      end
      subroutine g(value,i,j,k,l)
      implicit double precision (a-h, o-z)
      include 'cscf.h'
c
c     compute the two electon integral (ij|kl) over normalized
c     primitive 1s gaussians
c
      f0val = 0.0d0
      rab2 = (x(i)-x(j))**2 + (y(i)-y(j))**2 + (z(i)-z(j))**2
      rcd2 = (x(k)-x(l))**2 + (y(k)-y(l))**2 + (z(k)-z(l))**2
      facij = expnt(i)*expnt(j)/(expnt(i)+expnt(j))
      fackl = expnt(k)*expnt(l)/(expnt(k)+expnt(l))
      exijkl = exprjh(- facij*rab2 - fackl*rcd2)
      denom = (expnt(i)+expnt(j))*(expnt(k)+expnt(l)) *
     $        sqrt(expnt(i)+expnt(j)+expnt(k)+expnt(l))
      fac = (expnt(i)+expnt(j))*(expnt(k)+expnt(l)) /
     $        (expnt(i)+expnt(j)+expnt(k)+expnt(l))
c
      xp = (x(i)*expnt(i) + x(j)*expnt(j))/(expnt(i)+expnt(j))
      yp = (y(i)*expnt(i) + y(j)*expnt(j))/(expnt(i)+expnt(j))
      zp = (z(i)*expnt(i) + z(j)*expnt(j))/(expnt(i)+expnt(j))
      xq = (x(k)*expnt(k) + x(l)*expnt(l))/(expnt(k)+expnt(l))
      yq = (y(k)*expnt(k) + y(l)*expnt(l))/(expnt(k)+expnt(l))
      zq = (z(k)*expnt(k) + z(l)*expnt(l))/(expnt(k)+expnt(l))
      rpq2 = (xp-xq)**2 + (yp-yq)**2 + (zp-zq)**2
c
      call f0(f0val, fac*rpq2)
      value = (2.0d0 * pi**2.5d0 / denom) * exijkl * f0val *
     $    rnorm(i)*rnorm(j)*rnorm(k)*rnorm(l)
c
      end
      subroutine diagon(fock, orbs, evals, work, tester, iter)
      implicit double precision (a-h, o-z)
      include 'cscf.h'
      dimension fock(nnbfn), orbs(nbfn, nbfn), evals(nbfn),
     $     work(nbfn*nbfn)
      dimension ilifq(nbfn), work2(nbfn*nbfn)
c
      do 10 i = 1, nbfn
         ilifq(i) = (i-1)*nbfn
 10   continue
c
c     transform fock matrix from AO to MO basis
c
      call zsqua(nbfn, fock, work)
      call dgemm('n', 'n', nbfn, nbfn, nbfn, 1.0d0, 
     $     work, nbfn, orbs, nbfn, 0.0d0, work2, nbfn)
      call dgemm('t', 'n', nbfn, nbfn, nbfn, 1.0d0, 
     $     orbs, nbfn, work2, nbfn, 0.0d0, work, nbfn)
      call zfold(nbfn, fock, work)
c
      iop1 = 1
      iop2 = 2
      tester = 0.0d0
      do 20 i = 2, nbfn
         do 30 j = 1, i-1
            tester = max(tester, abs(fock(iky(i)+j)))
 30      continue
 20   continue
c
      shift = 0.3d0
      if (iter.ge.2) then
         do 40 i = nocc+1, nbfn
            fock(iky(i)+i) = fock(iky(i)+i) + shift
 40      continue
      endif
      thresh = min(0.0001d0,max(1.0d-12,tester*0.01d0))
      call jacobi(fock, iky, nbfn, orbs, ilifq, nbfn, evals, iop1,
     *     iop2,thresh)
      if (iter.ge.2) then
         do 50 i = nocc+1, nbfn
            evals(i) = evals(i) - shift
 50      continue
      endif
c
      end
      subroutine makeob(orbs, work)
      implicit double precision (a-h, o-z)
      include 'cscf.h'
      include 'msgtypesf.h'
      dimension orbs(nbfn,nbfn), work(nbfn,nbfn)
      dimension tmp1(nbfn), tmp2(nbfn)
c
c     generate set of orthonormal vectors by orthoging twice
c     a set of random vectors ... don't do this at home!
c     ... should really diagonalize the overlap to get sym adaption
c
      if (nodeid().eq.0) then
         call srand48(12345)
         do 10 i = 1, nbfn
            do 20 j = 1, nbfn
               work(j,i) = s(i,j)
               orbs(j,i) = drand48(0)
 20         continue
 10      continue
         call orthv2(nbfn, orbs, work, tmp1, tmp2)
         call orthv2(nbfn, orbs, work, tmp1, tmp2)
      endif
      call brdcst(99+MSGDBL, orbs, mdtob(nbfn*nbfn), 0)
c
      end
