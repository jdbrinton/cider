      subroutine diagon(fock, orbs, evals, work)
      implicit double precision (a-h, o-z)
      include 'cscf.h'
      dimension fock(nnbfn), orbs(nbfn, nbfn), evals(nbfn),
     $     work(nbfn, nbfn)
      dimension temp1(nbfn), temp2(nbfn)
c
c     Diagonalize the fock matrix. We use the eispack routine
c     rsg which solves the generalized symmetric eigenvalue
c     problem hc = esc, where h is the hamiltonian matrix, 
c     s is the overlap matrix, e the eigen-values and c the
c     eigen-vectors. First fill orbs with fock and work1 with s
c
      do 10 i = 1, nbfn
         do 20 j = 1, i
            ij = iky(i) + j
            orbs(i,j) = fock(ij)
            orbs(j,i) = fock(ij)
            work(j,i) = s(i,j)
            work(i,j) = work(j,i)
 20      continue
 10   continue
c
c     now call rsg to do the dirty work
c
      call rsg(nbfn,nbfn,orbs,work,evals,1,orbs,temp1,temp2,ierr)
      if (ierr.ne.0) then
         write(6,*) ' rsg failed when called by diagon ', ierr
         stop 99
      endif
c
      end
