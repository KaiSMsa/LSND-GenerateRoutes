#include "StdAfx.h"
#include "TriTas.h"

TriTas::TriTas(void)
{
}

TriTas::~TriTas(void)
{
}

void swap(int tab[],int a,int b)
{
 int hh;
 hh=tab[a];
 tab[a]=tab[b];
 tab[b]=hh;
}

void ordonner(int tab[],int taille,int valtri[],int nb,int ii)
{
 int j;
 if(nb==0)
 {
  if((2*ii+1>taille)||(valtri[tab[2*ii]]>=valtri[tab[2*ii+1]])) j=2*ii;
  else j=2*ii+1;
  if(valtri[tab[j]]>valtri[tab[ii]])
  {
   swap(tab,ii,j);
   if(j<=taille/2) ordonner(tab,taille,valtri,nb,j);
  }
 }
 else
 {
  if((2*ii+1>taille)||(valtri[tab[2*ii]]<=valtri[tab[2*ii+1]])) j=2*ii;
  else j=2*ii+1;
  if(valtri[tab[j]]<valtri[tab[ii]])
  {
   swap(tab,ii,j);
   if(j<=taille/2) ordonner(tab,taille,valtri,nb,j);
  }
 }
}

void detruire(int tab[],int valtri[],int nb,int p)
{
 int ii,j;
 ii=1;
 if(nb==0)
 {
  while(ii<=p/2)
  {
   if((2*ii==p)||(valtri[tab[2*ii]]>valtri[tab[2*ii+1]])) j=2*ii;
   else j=2*ii+1;
   if(valtri[tab[ii]]<valtri[tab[j]])
   {
	swap(tab,ii,j);
    ii=j;
   }
   else break;
  }
 }
 else
 {
  while(ii<=p/2)
  {
   if((2*ii==p)||(valtri[tab[2*ii]]<valtri[tab[2*ii+1]])) j=2*ii;
   else j=2*ii+1;
   if(valtri[tab[ii]]>valtri[tab[j]])
   {
	swap(tab,ii,j);
    ii=j;
   }
   else break;
  }
 }
}

void TriTas::triTabEntiers(int tab[],int taille,int valtri[],ordre o)
{
 int p,max;
 int nb = o;
 int val=1;
 for(p=taille/2;p>=1;p--)
  ordonner(tab,taille,valtri,nb,p);
 p=taille;
 while(p>val)
 {
  max=tab[1];
  tab[1]=tab[p];
  p=p-1;
  detruire(tab,valtri,nb,p);
  tab[p+1]=max;
 }
}


/*********************************************************
fin de la fonction tri_tas reservé aux entiers
*********************************************************/
/*******************************************************
					debut tri_tas (float)
********************************************************/

void ordonner_f(int tab[],int taille,float valtri[],int nb,int ii)
{
 int j;
 if(nb==0)
 {
  if((2*ii+1>taille)||(valtri[tab[2*ii]]>=valtri[tab[2*ii+1]])) j=2*ii;
  else j=2*ii+1;
  if(valtri[tab[j]]>valtri[tab[ii]])
  {
   swap(tab,ii,j);
   if(j<=taille/2) ordonner_f(tab,taille,valtri,nb,j);
  }
 }
 else
 {
  if((2*ii+1>taille)||(valtri[tab[2*ii]]<=valtri[tab[2*ii+1]])) j=2*ii;
  else j=2*ii+1;
  if(valtri[tab[j]]<valtri[tab[ii]])
  {
   swap(tab,ii,j);
   if(j<=taille/2) ordonner_f(tab,taille,valtri,nb,j);
  }
 }
}

void detruire_f(int tab[],float valtri[],int nb,int p)
{
 int ii,j;
 ii=1;
 if(nb==0)
 {
  while(ii<=p/2)
  {
   if((2*ii==p)||(valtri[tab[2*ii]]>valtri[tab[2*ii+1]])) j=2*ii;
   else j=2*ii+1;
   if(valtri[tab[ii]]<valtri[tab[j]])
   {
	swap(tab,ii,j);
    ii=j;
   }
   else break;
  }
 }
 else
 {
  while(ii<=p/2)
  {
   if((2*ii==p)||(valtri[tab[2*ii]]<valtri[tab[2*ii+1]])) j=2*ii;
   else j=2*ii+1;
   if(valtri[tab[ii]]>valtri[tab[j]])
   {
	swap(tab,ii,j);
    ii=j;
   }
   else break;
  }
 }
}
void TriTas::triTabReels(int tab[],int taille,int val,float valtri[],int nb)
{
 int p,max;
 for(p=taille/2;p>=1;p--)
  ordonner_f(tab,taille,valtri,nb,p);
 p=taille;
 while(p>val)
 {
  max=tab[1];
  tab[1]=tab[p];
  p=p-1;
  detruire_f(tab,valtri,nb,p);
  tab[p+1]=max;
 }
}
/*******************************************************
					fin tri_tas (float)
********************************************************/
