//Copyright 2018 Raphael Haucke
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//http ://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.

#include "GeneticAlgorithm.h"
#include "Engine/World.h"
#include "Phenotype.h"
#include "Genotype.h"
#include "Species.h"
#include "Innovation.h"
#include "Parameters.h"
#include "Queue.h"
#include "MyGameMode.h"



UGeneticAlgorithm::UGeneticAlgorithm()
{
}

void UGeneticAlgorithm::Initialize(int populationSize, int numInputs, int numOutputs, AMyGameMode* gameMode)
{
	m_iGeneration = 1;
	m_iPopSize = populationSize;
	m_iNextGenomeID = 0;
	m_Innovation = nullptr;
	m_iNextSpeciesID = 0;
	m_dBestFitnessEver = 0.0;
	m_dTotalAdjustedFitness = 0.0;
	m_dAverageAdjustedFitness = 0.0;
	m_GameMode = gameMode;
	m_Parameters = m_GameMode->GetParameters();

	//create population of start genomes
	for (int i = 0; i < m_iPopSize; ++i)
	{
		UGenome* NewGenome = NewObject<UGenome>();
		NewGenome->InitializeStandard(0, numInputs, numOutputs, m_GameMode);
		NewGenome->SetID(i);
		m_Genomes.Add(NewGenome);
		m_Genomes[i]->InitializeWeights();
		++m_iNextGenomeID;
	}

	//create the innovation list with the first genome
	m_Innovation = NewObject<UInnovation>(this);
	m_Innovation->Initialize(m_Genomes[0]->GetLinkGenesList(), m_Genomes[0]->GetNeuronGenesList());

	//create the network depth lookup table
	m_FSplitDepths = Split(0, 1, 0);
}

TArray<UNeuralNet*> UGeneticAlgorithm::Epoch(TArray<double>& vGenotypeFitness)
{
	if (vGenotypeFitness.Num() != m_Genomes.Num())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT(" GeneticAlgorithm Epoch() Not enough fittness scores"));
	}

	//ready for next generation
	ResetAndKill();

	//assign the restulting fitness of the game run to the respective genome
	for (int i = 0; i < m_Genomes.Num(); ++i)
	{
		m_Genomes[i]->SetFitness(vGenotypeFitness[i]);
	}

	//sort genomes and keep a record of the best performers
	SortAndRecord();

	//speciate
	SpeciateAndCalculateSpawnAmounts();

	//calc stats for records
	for (UGenome* curGenome : m_Genomes)
	{
		m_AvgNumLinksLastGen += curGenome->GetNumLinkGenes();
		m_AvgNumNeuronsLastGen += curGenome->GetNumNeuronGenes();
	}

	m_AvgNumLinksLastGen /= m_Genomes.Num();
	m_AvgNumNeuronsLastGen /= m_Genomes.Num();


	//the next generation of genomes
	TArray<UGenome*> NextGeneration;

	int NextGenSize = 0;
	UGenome* NextChild = nullptr;

	//go through all species
	for (USpecies* curSpecies : m_Species)
	{
		//check if already done with next generation
		if (NextGenSize < m_Parameters->iPopulationSize)
		{
			//species spawn amount is a double which needs to be rounded to an integer
			int NumToSpawn = Round(curSpecies->GetNumToSpawn());

			bool bChosenBestYet = false;

			//as long as current species is allowed to generate offspring
			while (NumToSpawn > 0)
			{
				//copy leader of current species (per species elitism) once
				if (!bChosenBestYet)
				{
					NextChild = NewObject<UGenome>(this, UGenome::StaticClass(), NAME_None, RF_NoFlags, curSpecies->GetLeader());
					bChosenBestYet = true;
				}
				else
				{
					//if the number of individuals in this species is only one then we can't crossover
					if (curSpecies->GetNumMembers() == 1)
					{
						NextChild = NewObject<UGenome>(this, UGenome::StaticClass(), NAME_None, RF_NoFlags, curSpecies->GetTopGenome());
					}
					//if greater than one we can use the crossover operator
					else
					{
						//select first parent
						UGenome* MotherGenome = NewObject<UGenome>(this, UGenome::StaticClass(), NAME_None, RF_NoFlags, curSpecies->GetTopGenome());
						//do we crossover?
						if (RandFloat() < m_Parameters->dCrossoverRate)
						{
							//select second parent
							UGenome* FatherGenome = NewObject<UGenome>(this, UGenome::StaticClass(), NAME_None, RF_NoFlags, curSpecies->GetTopGenome());
							int NumAttempts = m_Parameters->iCrossoverTries;

							//father needs to be different from mother
							while ((MotherGenome->GetID() == FatherGenome->GetID()) && (NumAttempts > 0))
							{
								FatherGenome = NewObject<UGenome>(this, UGenome::StaticClass(), NAME_None, RF_NoFlags, curSpecies->GetTopGenome());
								--NumAttempts;
							}

							//two different parents, do crossover
							if (MotherGenome->GetID() != FatherGenome->GetID())
							{
								NextChild = Crossover(MotherGenome, FatherGenome);
							}
						}
						//couldn't find partner, child is mother
						else
						{
							NextChild = MotherGenome;
						}

						//mutate the child
						if (NextChild->GetNumNeuronGenes() < m_Parameters->iMaxPermittedNeurons)
						{
							NextChild->MutateAddNode(*m_Innovation, m_Parameters->dChanceAddNode, m_Parameters->iNumTriesAddNode);
						}

						NextChild->MutateAddLink(*m_Innovation, m_Parameters->dChanceAddLink, m_Parameters->iNumTriesAddLink);
						NextChild->MutateWeights(m_Parameters->dMaxWeightMutationPower, m_Parameters->dWeightMutationRate, m_Parameters->dNewWeightChance);
						NextChild->ToggleLinkGenes(m_Parameters->dToggleLinkRate, m_Parameters->iNumTriesToggle);
						NextChild->ReenableLinkGenes(m_Parameters->dEnableLinkRate);

						//sort link genes of the new generation member
						NextChild->SortGenes();
					}
					//give the offspring its ID
					NextChild->SetID(m_iNextGenomeID);
					++m_iNextGenomeID;
				}
				//add child to new pop
				NextGeneration.Add(NextChild);
				--NumToSpawn;
				++NextGenSize;

				if (NextGenSize == m_Parameters->iPopulationSize)
				{
					NumToSpawn = 0;
				}
			}
		}
	}//next species

	 //if there is an underflow due to the rounding error and the amount
	 //of offspring falls short of the population size, additional children
	 //need to be created and added to the new population. This is achieved
	 //by using tournament selection over the entire population.
	if (NextGenSize < m_Parameters->iPopulationSize)
	{
		//calculate amount of additional children required
		int ChildrenRequired = m_Parameters->iPopulationSize - NextGenSize;

		while (ChildrenRequired > 0)
		{
			NextGeneration.Add(TournamentSelection(m_Parameters->iNumTriesForSelection));
			--ChildrenRequired;
		}
	}

	//delete last generation and assign the next one
	m_Genomes.Empty();
	m_Genomes = NextGeneration;

	//create phenotypes
	TArray<UNeuralNet*> TempNeuralNets;

	for (UGenome* genome : m_Genomes)
	{
		genome->CalculateNetDepth(m_FSplitDepths);
		UNeuralNet* TempNeuralNet = genome->CreatePhenotype();
		TempNeuralNets.Emplace(TempNeuralNet);
	}

	//generation done
	++m_iGeneration;

	return TempNeuralNets;
}


TArray<UGenome*> UGeneticAlgorithm::GetGenotypes()
{
	TArray<UGenome*> TempGenotypes;
	for (UGenome* genome : m_Genomes)
	{
		TempGenotypes.Add(genome);
	}
	return TempGenotypes;
}

void UGeneticAlgorithm::SpeciateAndCalculateSpawnAmounts()
{
	bool bAdded = false;

	//try to keep the number of species at iMaxNumberOfSpecies
	AdjustCompatibilityThreshold();

	//iterate through each genome and speciate
	for (UGenome* genome : m_Genomes)
	{
		//calculate its compatibility score with each species leader
		for (USpecies* species : m_Species)
		{
			UGenome* SpeciesLeader = species->GetLeader();
			double Compatibility = genome->GetCompatibilityScore(SpeciesLeader);

			//if this individual is similar to this species add to species
			if (Compatibility <= m_Parameters->dCompatibilityThreshold)
			{
				//let the genome know which species it's in
				genome->SetSpecies(species->GetSpeciesID());
				species->AddMember(genome);

				bAdded = true;

				break;
			}
		}

		if (!bAdded)
		{
			//couldn't find a compatible species so create a new one
			USpecies* NewSpecies = NewObject<USpecies>(this);
			NewSpecies->Initialize(genome, m_iNextSpeciesID, m_GameMode);
			genome->SetSpecies(m_iNextSpeciesID);
			++m_iNextSpeciesID;
			m_Species.Add(NewSpecies);
		}

		bAdded = false;
	}

	//all the genomes have been assigned so adjust species fitness
	AdjustSpeciesFitnesses();

	//calculate new adjusted total & average fitness for the population
	for (UGenome* genome : m_Genomes)
	{
		m_dTotalAdjustedFitness += genome->GetSpeciesFitness();
	}

	m_dAverageAdjustedFitness = m_dTotalAdjustedFitness / m_Genomes.Num();

	//calculate how many offspring each member of the population should spawn
	for (UGenome* genome : m_Genomes)
	{
		double Offspring = genome->GetSpeciesFitness() / m_dAverageAdjustedFitness;
		if (Offspring < 0.f)
		{
			Offspring = 0.f;
		}
		genome->SetSpawnAmount(Offspring);
	}

	//iterate through all the species and calculate how many offspring each species should spawn
	for (USpecies* species: m_Species)
	{
		species->CalculateSpawnAmount();
	}
}

void UGeneticAlgorithm::ResetAndKill()
{
	m_dTotalAdjustedFitness = 0.0;
	m_dAverageAdjustedFitness = 0.0;

	TQueue<USpecies*> toDelete;

	//purge the species
	for (USpecies* species : m_Species)
	{
		species->Purge();
		double BestFitness = species->GetBestFitness();
		int GenNoImprov = species->GetGensNoImprovement();

		//kill off species if not improving and if not the species which contains the best genome found so far
		if ((GenNoImprov > m_Parameters->iNumGensAllowedNoImprovement) && (BestFitness < m_dBestFitnessEver))
		{
			toDelete.Enqueue(species);
		}
	}

	USpecies* deleteItem;
	while (toDelete.Dequeue(deleteItem))
	{
		m_Species.Remove(deleteItem);
	}

	//delete phenotypes
	for (UGenome* genome : m_Genomes)
	{
		genome->DeletePhenotype();
	}
}

void UGeneticAlgorithm::AdjustCompatibilityThreshold()
{
	//if iSpeciesTarget < 1 then feature is disabled
	if (m_Parameters->iSpeciesTarget < 1)
	{
		return;
	}

	//10%
	double ThresholdModifier = 0.1;

	if (m_iGeneration > 1)
	{
		if (m_Species.Num() > m_Parameters->iSpeciesTarget)
		{
			m_Parameters->dCompatibilityThreshold += (m_Parameters->dCompatibilityThreshold * ThresholdModifier);
		}

		else if (m_Species.Num() < m_Parameters->iSpeciesTarget)
		{
			m_Parameters->dCompatibilityThreshold -= (m_Parameters->dCompatibilityThreshold * ThresholdModifier);
		}

		//if to low set to 10% of dCompatibilityThreshold
		if (m_Parameters->dCompatibilityThreshold < m_Parameters->dCompatibilityThreshold * ThresholdModifier)
		{
			m_Parameters->dCompatibilityThreshold = m_Parameters->dCompatibilityThreshold * ThresholdModifier;
		}
	}

	return;
}

void UGeneticAlgorithm::AdjustSpeciesFitnesses()
{
	for (USpecies* species : m_Species)
	{
		species->AdjustFitnessScores();
	}
}

UGenome* UGeneticAlgorithm::Crossover(UGenome* motherGenome, UGenome* fatherGenome)
{
	UGenome* FitterParent = nullptr;
	UGenome* OtherParent = nullptr;
	double MotherFitness = motherGenome->GetFitness();
	double FatherFitness = fatherGenome->GetFitness();
	int MotherSize = motherGenome->GetNumLinkGenes();
	int FatherSize = fatherGenome->GetNumLinkGenes();

	//if both have same fitness
	if (MotherFitness == FatherFitness)
	{
		//and same size
		if (MotherSize == FatherSize)
		{
			//choose random
			if (RandFloat() < 0.5f)
			{
				FitterParent = motherGenome;
				OtherParent = fatherGenome;
			}
			FitterParent = fatherGenome;
			OtherParent = motherGenome;
		}
		//else prefer smaller genome
		else if (MotherSize < FatherSize)
		{
			FitterParent = motherGenome;
			OtherParent = fatherGenome;
		}
		else if (FatherSize < MotherSize)
		{
			FitterParent = fatherGenome;
			OtherParent = motherGenome;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("GeneticAlgorithm Crossover error parent size"));
		}
	}
	//choose fitter parent
	else if (MotherFitness < FatherFitness)
	{
		FitterParent = fatherGenome;
		OtherParent = motherGenome;
	}
	else if (FatherFitness < MotherFitness)
	{
		FitterParent = motherGenome;
		OtherParent = fatherGenome;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("GeneticAlgorithm Crossover error parent fitness"));
	}

	TArray<FSNeuronGene> BabyNeurons;
	TArray<int> NecessaryNeuronIDs;

	TArray<FSLinkGene> BabyLinkGenes;

	//iterators for the current link genes
	TArray<FSLinkGene> FitParentLinks = FitterParent->GetLinkGenesList();
	TArray<FSLinkGene> OtherParentLinks = OtherParent->GetLinkGenesList();
	TArray<FSLinkGene>::TIterator CurrentFitterGene = FitParentLinks.CreateIterator();
	TArray<FSLinkGene>::TIterator CurrentOtherGene = OtherParentLinks.CreateIterator();

	FSLinkGene SelectedLinkGene;

	//if we should push the gene
	bool bPushGene = true;

	//select genes until we are done with both genomes
	while (CurrentFitterGene || CurrentOtherGene)
	{
		//reached the end of the worse genome, just add excess gene from the fitter one
		if (!CurrentOtherGene)
		{
			SelectedLinkGene = *CurrentFitterGene;
			++CurrentFitterGene;
			bPushGene = true;
		}
		//reached end of fitter genome, just increment the worse one until both ends are reached
		else if (!CurrentFitterGene)
		{
			++CurrentOtherGene;
			bPushGene = false;
		}
		//both IDs are the same
		else if (CurrentFitterGene->iInnovationID == CurrentOtherGene->iInnovationID)
		{
			//select randomly
			if (RandFloat() < 0.5)
			{
				SelectedLinkGene = *CurrentFitterGene;
			}
			else
			{
				SelectedLinkGene = *CurrentOtherGene;
			}

			++CurrentFitterGene;
			++CurrentOtherGene;
			bPushGene = true;
		}
		//fitter ID is smaller so we can select
		else if (CurrentFitterGene->iInnovationID < CurrentOtherGene->iInnovationID)
		{
			SelectedLinkGene = *CurrentFitterGene;
			++CurrentFitterGene;
			bPushGene = true;
		}
		//fitter ID is bigger, we musn't push it yet, because it could be a matching gene
		else if (CurrentFitterGene->iInnovationID > CurrentOtherGene->iInnovationID)
		{
			++CurrentOtherGene;
			bPushGene = false;
		}

		//add the selected link gene
		if (bPushGene)
		{
			if (SelectedLinkGene.bEnabled == false)
			{
				//75% chance for the gene to stay disabled
				if (RandFloat() < 0.25f)
				{
					SelectedLinkGene.bEnabled = true;
				}
			}
			BabyLinkGenes.Add(SelectedLinkGene);
		}

		//add neurons of the used link if they aren't already registered
		AddNeuronID(SelectedLinkGene.FromNeuron, NecessaryNeuronIDs);
		AddNeuronID(SelectedLinkGene.ToNeuron, NecessaryNeuronIDs);
	}//end while

	NecessaryNeuronIDs.Sort();

	for (int32 ID : NecessaryNeuronIDs)
	{
		BabyNeurons.Add(m_Innovation->CreateNeuronFromID(ID));
	}

	UGenome* BabyGenome = NewObject<UGenome>();
	BabyGenome->InitializeCustom(-1, BabyNeurons, BabyLinkGenes, motherGenome->GetNumInputs(), motherGenome->GetNumOutputs(), m_GameMode);

	return BabyGenome;
}

UGenome* UGeneticAlgorithm::TournamentSelection(int numTries)
{
	double BestFitnessSoFar = 0;

	int ChosenGenomeID = 0;

	//select NumComparisons members from the population at random testing against the best found so far
	for (int i = 0; i < numTries; ++i)
	{
		int ThisTry = RandInt(0, m_Genomes.Num() - 1);

		if (m_Genomes[ThisTry]->GetFitness() > BestFitnessSoFar)
		{
			ChosenGenomeID = ThisTry;

			BestFitnessSoFar = m_Genomes[ThisTry]->GetFitness();
		}
	}

	return m_Genomes[ChosenGenomeID];
}

void UGeneticAlgorithm::SortAndRecord()
{
	//sort the genomes according to their unadjusted (no fitness sharing) fitness
	m_Genomes.Sort();

	//is the best genome this generation the best ever?
	if (m_Genomes[0]->GetFitness() >= m_dBestFitnessEver)
	{
		m_dBestFitnessEver = m_Genomes[0]->GetFitness();
		m_BestGenomeEver = NewObject<UGenome>(this, UGenome::StaticClass(), NAME_None, RF_NoFlags, m_Genomes[0]);
	}

	//keep a record of the n best genomes
	StoreBestGenomes();
}

void UGeneticAlgorithm::StoreBestGenomes()
{
	//clear old record
	m_BestGenomes.Empty();

	for (int i = 0; i < m_Parameters->iNumBestOrganisms; ++i)
	{
		m_BestGenomes.Add(m_Genomes[i]);
	}
}

TArray<UNeuralNet*> UGeneticAlgorithm::GetLastGenerationsBestPhenotypes()
{
	TArray<UNeuralNet*> BestPhenotypes;

	for (UGenome* curGenome : m_BestGenomes)
	{
		curGenome->CalculateNetDepth(m_FSplitDepths);
		BestPhenotypes.Add(curGenome->CreatePhenotype());
	}
	return BestPhenotypes;
}

UNeuralNet* UGeneticAlgorithm::GetBestPhenotype()
{
	m_BestGenomeEver->CalculateNetDepth(m_FSplitDepths);
	UNeuralNet* BestPhenotype = m_BestGenomeEver->CreatePhenotype();

	return BestPhenotype;
}

void UGeneticAlgorithm::SetFitness(int genome, double fitness)
{
	m_Genomes[genome]->SetFitness(fitness);
}

bool UGeneticAlgorithm::AddNeuronID(int neuronID, TArray<int> &vNeurons)
{
	bool bFoundID = false;

	for (int i = 0; i < vNeurons.Num(); ++i)
	{
		if (vNeurons[i] == neuronID)
		{
			bFoundID = true;
			return true;
		}
	}

	if (!bFoundID)
	{
		vNeurons.Add(neuronID);
	}
	return false;
}

TArray<FSplitDepth> UGeneticAlgorithm::Split(double low, double high, int depth)
{
	static TArray<FSplitDepth> vSplits;
	double span = high - low;

	vSplits.Add(FSplitDepth(low + span / 2, depth + 1));

	//calculates the SlitY-values to a depth of 6 hidden layers so to calc the net depth
	//you just have to check each neuron for its SplitY and compare it to the list
	if (depth > 6)
	{
		return vSplits;
	}
	else
	{
		Split(low, low + span / 2, depth + 1);
		Split(low + span / 2, high, depth + 1);

		return vSplits;
	}
}

FString UGeneticAlgorithm::GetGenomeStats()
{
	double AvgNumLinks = m_AvgNumLinksLastGen;
	double AvgNumNeurons = m_AvgNumNeuronsLastGen;

	m_AvgNumLinksLastGen = 0.0;
	m_AvgNumNeuronsLastGen = 0.0;

	FString stats = FString::SanitizeFloat(AvgNumLinks) + ";" + FString::SanitizeFloat(AvgNumNeurons);
	return stats;
}