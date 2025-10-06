#pragma once
class TriTas
{
public:
	enum ordre{
		croissant,
		decroissant
	};
public:
	TriTas(void);
	~TriTas(void);
    static void triTabEntiers(int tabIndice[], int tailleIndice, int valTri[], ordre);
	/********** pour les float***************************************************/
    static void triTabReels(int tab[],int taille,int val,float valtri[],int nb);
};

