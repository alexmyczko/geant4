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
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
//
// $Id: G4GeometryManager.cc,v 1.7 2002/02/26 18:26:39 gcosmo Exp $
// GEANT4 tag $Name: geant4-04-00-patch-02 $
//
// class G4GeometryManager
//
// Implementation
//
// Author:
// 26.07.95 P.Kent Initial version, including optimisation Build

#include "G4Timer.hh"
#include "G4GeometryManager.hh"
#include "g4std/iomanip"

// Close geometry - perform sanity checks and optionally Build optimisation
// for placed volumes (always built for replicas & parameterised)
// NOTE: Currently no sanity checks
G4bool G4GeometryManager::CloseGeometry(G4bool pOptimise, G4bool verbose)
{
  if (!fIsClosed)
  {
     BuildOptimisations(pOptimise, verbose);
     fIsClosed=true;
  }
  return true;
}

void G4GeometryManager::OpenGeometry()
{
  if (fIsClosed)
  {
     DeleteOptimisations();
     fIsClosed=false;
  }
}

// Static class variable: ptr to single instance of class
G4GeometryManager* G4GeometryManager::fgInstance = 0;

G4GeometryManager* G4GeometryManager::GetInstance()
{
   static G4GeometryManager worldManager;
   if (!fgInstance)
   {
     fgInstance = &worldManager;
   }
   return fgInstance;    
}


// Constructor. Set the geometry to be open
G4GeometryManager::G4GeometryManager() 
{
   fIsClosed=false;
}

//
// Create optimisation info. Build all voxels if allOpts=true
// else only for replicated volumes
//
void G4GeometryManager::BuildOptimisations(G4bool allOpts, G4bool verbose)
{
   G4Timer timer;
   G4Timer allTimer;
   G4std::vector<G4SmartVoxelStat> stats;
   if (verbose) allTimer.Start();

   G4LogicalVolumeStore *Store;
   G4LogicalVolume *volume;
   G4SmartVoxelHeader *head;
   G4int nVolumes,n;
   Store=G4LogicalVolumeStore::GetInstance();
   nVolumes=Store->size();
   for (n=0;n<nVolumes;n++)
   {
      volume=(*Store)[n];
      // For safety, check if there are any existing voxels and delete before
      // replacement
      head = volume->GetVoxelHeader();
      if (head) 
      {
        delete head;
        volume->SetVoxelHeader(0);
      }
      if ((volume->GetNoDaughters()>=kMinVoxelVolumesLevel1&&allOpts) ||
          (volume->GetNoDaughters()==1 &&
           volume->GetDaughter(0)->IsReplicated()==true))
      {
#ifdef G4GEOMETRY_VOXELDEBUG
        G4cout << "**** G4GeometryManager::BuildOptimisations" << G4endl
               << "     Examining logical volume name = "
               << volume->GetName() << G4endl;
#endif
        head = new G4SmartVoxelHeader(volume);
        if (head)
        {
           volume->SetVoxelHeader(head);
        }
        else
        {
           G4Exception("G4GeometryManager::BuildOptimisations voxelheader new failed");
        }
        if (verbose)
        {
           timer.Stop();
           stats.push_back( G4SmartVoxelStat( volume, head,
                                              timer.GetSystemElapsed(),
                                              timer.GetUserElapsed()   ) );
        }
     }
     else
     {
        // Don't create voxels for this node
#ifdef G4GEOMETRY_VOXELDEBUG
        G4cout << "**** G4GeometryManager::BuildOptimisations" << G4endl
               << "     Skipping logical volume name = " << volume->GetName()
               << G4endl;
#endif
     }
  }
  if (verbose)
  {
     allTimer.Stop();
     ReportVoxelStats( stats, allTimer.GetSystemElapsed()
                            + allTimer.GetUserElapsed() );
  }
}

// Remove all optimisation info
//
// Process:
//
// Loop over all logical volumes, deleting non-null voxels ptrs

void G4GeometryManager::DeleteOptimisations()
{
   G4LogicalVolumeStore *Store=G4LogicalVolumeStore::GetInstance();
   G4LogicalVolume *volume;
   G4SmartVoxelHeader *head;
   G4int nVolumes,n;
   nVolumes=Store->size();
   for (n=0;n<nVolumes;n++)
   {
      volume=(*Store)[n];
      head=volume->GetVoxelHeader();
      if (head)
      {
        delete head;
        volume->SetVoxelHeader(0);
      }
   }
}

//
// ReportVoxelStats
//

void
G4GeometryManager::ReportVoxelStats( G4std::vector<G4SmartVoxelStat> &stats,
                                     G4double totalCpuTime )
{
  G4cout << "G4GeometryManager::ReportVoxelStats -- Voxel Statistics"
         << G4endl << G4endl;
 
  //
  // Get total memory use
  //
  G4int i, nStat = stats.size();
  G4long totalMemory = 0;
 
  for( i=0;i<nStat;++i ) totalMemory += stats[i].GetMemoryUse();
 
  G4cout << "    Total memory consumed for geometry optimisation:   "
         << totalMemory/1024 << " kByte" << G4endl;
  G4cout << "    Total CPU time elapsed for geometry optimisation: " 
         << G4std::setprecision(2) << totalCpuTime << " seconds" << G4endl;
 
  //
  // First list: sort by total CPU time
  //
  G4std::sort( stats.begin(), stats.end(), G4SmartVoxelStat::ByCpu() );
         
  G4int nPrint = nStat > 10 ? 10 : nStat;

  if (nPrint)
  {
    G4cout << "\n    Voxelisation: top CPU users:" << G4endl;
    G4cout << "    Percent   Total CPU    System CPU       Memory  Volume\n"
           << "    -------   ----------   ----------     --------  ----------"
           << G4endl;
    //         12345678901.234567890123.234567890123.234567890123k .
  }

  for(i=0;i<nPrint;++i)
  {
    G4double total = stats[i].GetTotalTime();
    G4double system = stats[i].GetSysTime();
    G4double perc = 0.0;

    if (system < 0) system = 0.0;
    if ((total < 0) || (totalCpuTime < perMillion))
      total = 0;
    else
      perc = total*100/totalCpuTime;

    G4cout << G4std::setprecision(2) 
           << G4std::setiosflags(G4std::ios::fixed|G4std::ios::right)
           << G4std::setw(11) << perc
           << G4std::setw(13) << total
           << G4std::setw(13) << system
           << G4std::setw(13) << (stats[i].GetMemoryUse()+512)/1024
           << "k " << G4std::setiosflags(G4std::ios::left)
           << stats[i].GetVolume()->GetName()
           << G4std::resetiosflags(G4std::ios::floatfield|G4std::ios::adjustfield)
           << G4std::setprecision(6)
           << G4endl;
  }
 
  //
  // Second list: sort by memory use
  //
  G4std::sort( stats.begin(), stats.end(), G4SmartVoxelStat::ByMemory() );
 
  if (nPrint)
  {
    G4cout << "\n    Voxelisation: top memory users:" << G4endl;
    G4cout << "    Percent     Memory      Heads    Nodes   Pointers    Total CPU    Volume\n"
           << "    -------   --------     ------   ------   --------   ----------    ----------"
           << G4endl;
    //         12345678901.2345678901k .23456789.23456789.2345678901.234567890123.   .
  }

  for(i=0;i<nPrint;++i)
  {
    G4long memory = stats[i].GetMemoryUse();
    G4double totTime = stats[i].GetTotalTime();
    if (totTime < 0) totTime = 0.0;

    G4cout << G4std::setprecision(2) 
           << G4std::setiosflags(G4std::ios::fixed|G4std::ios::right)
           << G4std::setw(11) << G4double(memory*100)/G4double(totalMemory)
           << G4std::setw(11) << memory/1024 << "k "
           << G4std::setw( 9) << stats[i].GetNumberHeads()
           << G4std::setw( 9) << stats[i].GetNumberNodes()
           << G4std::setw(11) << stats[i].GetNumberPointers()
           << G4std::setw(13) << totTime << "    "
           << G4std::setiosflags(G4std::ios::left)
           << stats[i].GetVolume()->GetName()
           << G4std::resetiosflags(G4std::ios::floatfield|G4std::ios::adjustfield)
           << G4std::setprecision(6)
           << G4endl;
  }
}