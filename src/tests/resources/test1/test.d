// ***************
// B_Announce_Herold 
// ***************

func void B_Announce_Herold ()
{
	var int randy;
	if ( C_BodystateContains(self, BS_SIT) )
	{
		AI_StandUp		(self);
		B_TurnToNpc		(self,	hero);
	};
	
	// ------ NSC steckt ggf. Waffe weg ------
	AI_RemoveWeapon (self);
	
	CreateInvItem		(self,	Fakescroll);
	AI_UseItemToState	(self,	Fakescroll,	1);

	AI_Output (self ,self,"DIA_Herold_Announce_04_00"); //Hört, ihr Bewohner von Khorinis! Auf ausdrückliche Anordnung des ehrenwerten Lord Hagens ergeht folgender Erlass.
	
	if (Kapitel <= 2)
	{
		randy = Hlp_Random (5);
		if (randy == 0)
		{
			AI_Output (self ,self,"DIA_Herold_Announce_04_01"); //Aufgrund der allgemeinen Lage zu eurem eigenen Schutz sind die Wälder und Wildnis in der Umgebung der Stadt zu meiden!
			AI_Output (self ,self,"DIA_Herold_Announce_04_02"); //Des weiteren ist jeglicher Kontakt zu den aufständischen Bauernschaften in den umliegenden Gebieten strengstens untersagt.
		}
		else if (randy == 1)
		{
			AI_Output (self ,self,"DIA_Herold_Announce_04_03"); //Ab sofort wird Lord Andre den Oberbefehl für unsere Miliz übernehmen.
			AI_Output (self ,self,"DIA_Herold_Announce_04_04"); //Alle Bürger dieser Stadt, die über kämpferische Fähigkeiten verfügen, werden hiermit angehalten, der königlichen Miliz beizutreten.
		}
		else if (randy == 2)
		{
			AI_Output (self ,self,"DIA_Herold_Announce_04_05"); //Die Sicherheitsmaßnahmen zum Schutz des oberen Viertels werden noch weiter verstärkt.
			AI_Output (self ,self,"DIA_Herold_Announce_04_06"); //Die Torwachen sind angewiesen, das Gesetz in aller Strenge auszulegen, um unerlaubtes Eindringen zu verhindern.
		}
		else if (randy == 3)
		{
			AI_Output (self ,self,"DIA_Herold_Announce_04_07"); //In allen Städten und Landstrichen des Reiches ist ab sofort das Kriegsrecht ausgerufen.
			AI_Output (self ,self,"DIA_Herold_Announce_04_08"); //Alle bürgerlichen Richter haben mit sofortiger Wirkung ihre Aufgaben an die königlichen Paladine zu übertragen.
			AI_Output (self ,self,"DIA_Herold_Announce_04_09"); //Der ehrenwerte Lord Andre ist angewiesen worden, jegliches Vergehen oder Widerstand gegen die Garde des Königs aufs Schwerste zu bestrafen.
			AI_Output (self ,self,"DIA_Herold_Announce_04_10"); //Jeder Bewohner von Khorinis, der sich eines Verbrechens schuldig macht, hat sich umgehend bei Lord Andre zu melden.
		}
		else
		{
			AI_Output (self ,self,"DIA_Herold_Announce_04_11"); //Jeder Bewohner der Stadt hat sich auf Grund der drohenden Gefahr eines Orkangriffs entsprechend vorzubereiten.
			AI_Output (self ,self,"DIA_Herold_Announce_04_12"); //Jeder hat sich ab sofort in den Fertigkeiten des Kampfes zu üben und sich angemessen zu bewaffnen.
		};
	}
	else if (Kapitel == 3)
	{
		IF (MIS_RescueBennet != LOG_SUCCESS)
		{
			AI_Output (self ,self,"DIA_Herold_Announce_04_13"); //Der Söldnerschmied Bennet, der den Paladin Lothar heimtückisch ermordet hat, ist im Namen Innos' verurteilt worden.
			AI_Output (self ,self,"DIA_Herold_Announce_04_14"); //Das Urteil lautet Tod durch den Strick. Die Vollstreckung erfolgt in wenigen Tagen.
		}
		else
		{
			AI_Output (self ,self,"DIA_Herold_Announce_04_15"); //Der Schmied Bennet wird freigesprochen, da seine Unschuld von einem klugen Berater Lord Hagens bewiesen wurde.
		};	
	}
	else if (Kapitel == 4)
	{
			AI_Output (self ,self,"DIA_Herold_Announce_04_16"); //Den aufkommenden Gerüchten über das Erscheinen von Drachen im Minental von Khorinis ist kein Glaube zu schenken.
			AI_Output (self ,self,"DIA_Herold_Announce_04_17"); //Die Gerüchte sind vom Feind gestreut worden, um das tapfere Volk von Myrtana in Angst und Schrecken zu versetzen.
			AI_Output (self ,self,"DIA_Herold_Announce_04_18"); //Um die lächerlichen Behauptungen zu widerlegen, ist ein Trupp tapferer Paladine unter dem Befehl eines Ortskundigen ins Minental aufgebrochen.
	}
	else	//Kapitel 5
	{
		randy = Hlp_Random (2);
		if (randy == 0)
		{
			AI_Output (self ,self,"DIA_Herold_Announce_04_19"); //Die Drachen, die das Land bedrohten, wurden von tapferen Männern unter dem Befehl von Lord Hagen besiegt.
			AI_Output (self ,self,"DIA_Herold_Announce_04_20"); //Bald wird auch König Rohbar das Land von den Orks befreit haben und das Königreich wird in neuem Glanz aufblühen!
		}
		else
		{
			AI_Output (self ,self,"DIA_Herold_Announce_04_21"); //Lord Andre erhält mit sofortiger Wirkung den Oberbefehl über die Stadt Khorinis.
			AI_Output (self ,self,"DIA_Herold_Announce_04_22"); //Lord Hagen hat verkündet, nun selbst ins Minental zu ziehen, um dafür zu sorgen, dass sein Schiff mit dem magischen Erz beladen werden kann.
		};
	};
	AI_UseItemToState	(self,	Fakescroll,	-1);
};


/* 

			AI_Output (self ,self,"DIA_Herold_Announce_04_00"); //Hört Bürger von Khorinis den Beschluß des ehrenwerten Lord Hagen!
			AI_Output (self ,self,"DIA_Herold_Announce_04_02"); //An alle Bürger von Khorinis, vernehmt Lord Hagens Anordnungen zum Schutze unserer Stadt. 
			AI_Output (self ,self,"DIA_Herold_Announce_04_09"); //Hört ihr Bewohner von Khorinis und vernehmt das königliche Dekret zur Aufrechterhaltung der öffentlichen Ordnung.
			AI_Output (self ,self,"DIA_Herold_Announce_04_09"); //An alle Bewohner von Khorinis, vernehmt Lord Hagens Anordnungen zum Schutze unserer Stadt. 





Kapitel 3 Spieler kommt aus dem Minental und hat mit Lord Hagen gesprochen
Hört ihr Büger von Khorinis. Ein Bote der Paladine brachte Kunde aus dem Minental
Unsere tapferen Paladine halten den Orks wehrhaft stand und erschliessen neue Minen für das Königreich!

Kapitel 3 Bennet ist im Knast
Hört ihr Bürger von Khorinis
Der Schmied Bennet, der den ehrenwerten Paladin Lothar heimtückisch ermordert hat, ist im Namen Innos verurteilt worden. 
Das Urteil lautet Tod durch den Strick.

Kapitel 3 Bennet ist wieder frei
Hört ihr Bürger,
Der Schmied Bennet wird freigesprochen, da seine Unschuld von einem klugen Berater Lord Hagens bewiesen wurde. 
So ist es Innos Wille und nach seiner Gerechtigkeit handeln wir. 
 
Kapitel 5 Start Drachen sind tot
Hört Bürger von Khorinis
Die Drachen die das Land bedrohten wurden von tapferen Männern unter dem Befehl von Lord Hagen besiegt.   
Bald wird auch König Rohbar das Land von den Orks befreien und das Königreich wird in neuem Glanz aufblühen!


Kapitel 5 Kurz vor Schluß (Spieler war in der Biblothek) 
Der ehrenwerte Lord Hagen hat verkündet nun selbst ins Minental zu ziehen um dafür zu sorgen das sein Schiff mit dem Erz beladen werden kann.  

*/
