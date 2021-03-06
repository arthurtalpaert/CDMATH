C*  This file is part of MED.
C*
C*  COPYRIGHT (C) 1999 - 2013  EDF R&D, CEA/DEN
C*  MED is free software: you can redistribute it and/or modify
C*  it under the terms of the GNU Lesser General Public License as published by
C*  the Free Software Foundation, either version 3 of the License, or
C*  (at your option) any later version.
C*
C*  MED is distributed in the hope that it will be useful,
C*  but WITHOUT ANY WARRANTY; without even the implied warranty of
C*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
C*  GNU Lesser General Public License for more details.
C*
C*  You should have received a copy of the GNU Lesser General Public License
C*  along with MED.  If not, see <http://www.gnu.org/licenses/>.
C*

C ******************************************************************************
C * - Nom du fichier : test3.f
C *
C * - Description : lecture des informations sur les maillages dans un fichier
C*                  MED.
C *
C ******************************************************************************
      program test3
C     
      implicit none
      include 'med.hf'
C
C
      integer       cret,fid,cres,type,cnu
      character*64  maa
      character*80  nomu
      character*200 desc
      integer       nmaa,i,mdim,edim,nstep,stype,atype
      character*16 nomcoo(2)
      character*16 unicoo(2)
      character*16 dtunit
      
C ** Ouverture du fichier en lecture seule
      call mfiope(fid,'test2.med',MED_ACC_RDONLY, cret)
      print *,cret
      if (cret .ne. 0 ) then
         print *,'Erreur ouverture du fichier en lecture'
         call efexit(-1)
      endif      

C ** lecture du nombre de maillage                      **
      call mmhnmh(fid,nmaa,cret)
      print *,cret
      if (cret .ne. 0 ) then
         print *,'Erreur lecture du nombre de maillage'
         call efexit(-1)
      endif      
      print *,'Nombre de maillages = ',nmaa

C ** lecture des infos sur les maillages : **
C ** - nom, dimension, type,description
C ** - options : nom universel, dimension de l'espace
      do i=1,nmaa  
         call mmhmii(fid,i,maa,edim,mdim,type,desc,
     &               dtunit,stype,nstep,atype,
     &               nomcoo,unicoo,cret)
         call mmhunr(fid,maa,nomu,cnu)
         print *,cret
         if (cret .ne. 0 ) then
            print *,'Erreur acces au maillage'
            call efexit(-1)
         endif      
         print '(A,I1,A,A4,A,I1,A,A65,A65)','maillage '
     &        ,i,' de nom ',maa,' et de dimension ',mdim,
     &        ' de description ',desc
         if (type.eq.MED_UNSTRUCTURED_MESH) then
            print *,'Maillage non structure'
         else
            print *,'Maillage structure'
         endif
         print *,'Dimension espace ', edim
         print *,'Dimension espace ', mdim
         if (cnu.eq.0) then
            print *,'Nom universel : ',nomu
         else
            print *,'Pas de nom universel'
         endif
         print *,'dt unit = ', dtunit
         print *,'sorting type =', stype
         print *,'number of computing step =', nstep
         print *,'coordinates axis type =', atype
         print *,'coordinates axis name =', nomcoo
         print *,'coordinates axis units =', unicoo
      enddo         
         
C **  fermeture du fichier
      call mficlo(fid,cret)
      print *,cret
      if (cret .ne. 0 ) then
         print *,'Erreur fermeture du fichier'
         call efexit(-1)
      endif      
C
      end 

