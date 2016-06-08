//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * authors in the GEANT4 collaboration.                             *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
//
// $Id: G4DistributionGenerator.hh,v 1.6 2001/10/04 20:00:39 hpw Exp $
// GEANT4 tag $Name: geant4-04-00 $
//
// -------------------------------------------------------------------
//      GEANT 4 class file --- Copyright CERN 1998
//      CERN Geneva Switzerland
//
//
//      File name:     G4DistributionGenerator.hh 
//
//      Author:        Maria Grazia Pia (pia@genova.infn.it)
// 
//      Creation date: 12 May 1998
//
//      Modifications: 
// -------------------------------------------------------------------

#ifndef G4DISTRIBUTIONGENERATOR_HH
#define G4DISTRIBUTIONGENERATOR_HH

#include "g4std/vector" 

#include "globals.hh"
#include "Randomize.hh"

class G4DistributionGenerator
{  

public:

  // Constructor
  G4DistributionGenerator(G4std::vector<G4double>& x,
  			  G4std::vector<G4double>& values);
  G4DistributionGenerator();

  // Destructor
  ~G4DistributionGenerator();

  G4double Generate(G4double ranflat);

private:

  // Hide assignment operator as private 
  //  G4DistributionGenerator& operator=(const G4DistributionGenerator &right);

  // Copy constructor
  //  G4DistributionGenerator(const G4DistributionGenerator& );

  G4std::vector<G4double> _x;
  G4std::vector<G4double> _cumProb;

};
 
#endif