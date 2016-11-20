#include "benchmark.h"
#include "problemparameters.h"
#include "solver.h"
#include "board.h"
#include <QThread>
#include <QTime>
#include <iostream>

Benchmark::Benchmark(ProblemParameters * params)
{
    this->params = params;
    thread = new QThread;
    moveToThread(thread);

    connect(thread,
            SIGNAL(started()),
            this,
            SLOT(run()));
}

Benchmark::~Benchmark()
{
    delete thread;
    delete params;
}

void Benchmark::start()
{
    thread->start();
}

void Benchmark::run()
{
    totalBadSpots = 0;
    totalProbabilities = 0.0;
    totalRupees = 0;
    totalClicks = 0;
    totalSetupTime = 0;
    totalRunTime = 0;
    knownBoard = new DugType::DugType*[params->height];
    for(int y = 0; y < params->height; y++)
    {
        knownBoard[y] = new DugType::DugType[params->width];
    }
    QTime timer;
    timer.start();
    for(int i = 0; i < 1000; i++)
    {
        for(int y = 0; y < params->height; y++)
        {
            std::fill(knownBoard[y], knownBoard[y] + params->width, DugType::DugType::undug);
        }
        board = new Board(params);
        solver = new Solver(params);
        probabilityArray = solver->getProbabilityArray();

        singleRun();

        delete board;
        delete solver;


    }
    //    std::cout << totalBadSpots << "\t" << totalProbabilities << std::endl;
    //    std::cout << totalClicks / 1000.0 << std::endl;
    for(double key: probabilityCount.keys())
    {
        std::cout <<
                     key <<
                     "\t" <<
                     probabilityCount.value(key) <<
                     "\t" <<
                     (double)probabilityGoneBad.value(key)/probabilityCount.value(key) <<
                     std::endl;
    }
    delete[] knownBoard;
    thread->exit();
    emit done();

}

void Benchmark::singleRun()
{
    QTime timer;
    DugType::DugType newSpot;
    double lowestprobability;
    int bestX;
    int bestY;
    int sumbadspots = 0;
    double sumProbabilities = 0.0;
    int rupees = 0;
    int clicks = 0;
    int setupTime = 0;
    int runTime = 0;
    int individualRunTime;
    int individualSetupTime;
    timer.start();
    solver->partitionCalculate();
    runTime += timer.elapsed();
    while(!board->hasWon())
    {
        lowestprobability = 1.0;
        for(int y = 0; y < params->height; y++)
        {
            for(int x = 0; x < params->width; x++)
            {
                if(knownBoard[y][x] == DugType::DugType::undug &&
                        probabilityArray[y][x] < lowestprobability)
                {
                    lowestprobability = probabilityArray[y][x];
                    bestX = x;
                    bestY = y;
                }
            }
        }
        clicks++;
        sumProbabilities += lowestprobability;
        if(!probabilityCount.contains(lowestprobability))
        {
            probabilityCount.insert(lowestprobability, 0);
            probabilityGoneBad.insert(lowestprobability, 0);
        }
        probabilityCount.insert(lowestprobability, probabilityCount.value(lowestprobability) + 1);
        newSpot = board->getCell(bestX, bestY);
        knownBoard[bestY][bestX] = newSpot;
        if(newSpot == DugType::DugType::bomb)
        {
            sumbadspots++;
            probabilityGoneBad.insert(lowestprobability, probabilityGoneBad.value(lowestprobability) + 1);
            break;
        }
        else if(newSpot == DugType::DugType::rupoor)
        {
            sumbadspots++;
            probabilityGoneBad.insert(lowestprobability, probabilityGoneBad.value(lowestprobability) + 1);
            rupees = std::max(rupees - 10, 0);
        }
        else if(newSpot == DugType::DugType::green)
        {
            rupees += 1;
        }

        else if(newSpot == DugType::DugType::blue)
        {
            rupees += 5;
        }
        else if(newSpot == DugType::DugType::red)
        {
            rupees += 20;
        }
        else if(newSpot == DugType::DugType::silver)
        {
            rupees += 100;
        }
        else if(newSpot == DugType::DugType::green)
        {
            rupees += 300;
        }
        timer.restart();
        solver->setCell(bestX, bestY, newSpot);
        individualSetupTime = timer.elapsed();
        setupTime += individualSetupTime;
        timer.restart();
        solver->partitionCalculate();
        individualRunTime = timer.elapsed();
        runTime += individualRunTime;
    }
    totalClicks += clicks;
    totalBadSpots += sumbadspots;
    totalProbabilities += sumProbabilities;
    totalRupees += rupees;
    totalRunTime += runTime;
    totalSetupTime += setupTime;

    //    std::cout << sumbadspots << "\t" << sumProbabilities << std::endl;
    //    std::cout << clicks << "\t" << runTime << std::endl;
}
