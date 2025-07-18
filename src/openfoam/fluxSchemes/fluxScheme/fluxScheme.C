/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2019 Synthetik Applied Technologies
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is derivative work of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "fluxScheme.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(fluxScheme, 0);
    defineRunTimeSelectionTable(fluxScheme, dictionary);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::fluxScheme::fluxScheme(const fvMesh& mesh)
:
    regIOobject
    (
        IOobject
        (
            "fluxScheme",
            mesh.time().timeName(),
            mesh
        )
    ),
    mesh_(mesh)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::fluxScheme::~fluxScheme()
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::fluxScheme::clear()
{
    own_.clear();
    nei_.clear();
    Uf_.clear();
    rhoOwn_.clear();
    rhoNei_.clear();
}

void Foam::fluxScheme::createSavedFields()
{
    if (own_.valid())
    {
        return;
    }
    own_ = tmp<surfaceScalarField>
    (
        new surfaceScalarField
        (
            IOobject
            (
                "fluxScheme::own",
                mesh_.time().timeName(),
                mesh_
            ),
            mesh_,
            dimensionedScalar("1", dimless, 1.0)
        )
    );
    nei_ = tmp<surfaceScalarField>
    (
        new surfaceScalarField
        (
            IOobject
            (
                "fluxScheme::nei",
                mesh_.time().timeName(),
                mesh_
            ),
            mesh_,
            dimensionedScalar("-1", dimless, -1.0)
        )
    );
    Uf_ = tmp<surfaceVectorField>
    (
        new surfaceVectorField
        (
            IOobject
            (
                "fluxScheme::Uf",
                mesh_.time().timeName(),
                mesh_
            ),
            mesh_,
            dimensionedVector("0", dimVelocity, Zero)
        )
    );
}

Foam::tmp<Foam::surfaceVectorField> Foam::fluxScheme::Uf() const
{
    if (Uf_.valid())
    {
        return Uf_;
    }
    return tmp<surfaceVectorField>
    (
        new surfaceVectorField
        (
            IOobject
            (
                "fluxScheme::Uf",
                mesh_.time().timeName(),
                mesh_,
                IOobject::NO_READ,
                IOobject::NO_WRITE,
                false
            ),
            mesh_,
            dimensionedVector("0", dimVelocity, Zero)
        )
    );
}


void Foam::fluxScheme::update
(
    const volScalarField& rho,
    const volVectorField& rhoU,
    const volScalarField& e,
    const volScalarField& rPsi,
    const volScalarField& c,
    surfaceScalarField& phi,
    surfaceScalarField& rhoPhi,
    surfaceVectorField& rhoUPhi,
    surfaceScalarField& rhoEPhi
)
{
    createSavedFields();

    rhoOwn_ = fvc::interpolate(rho, own_(), scheme("rho"));
    rhoNei_ = fvc::interpolate(rho, nei_(), scheme("rho"));

    surfaceVectorField rhoUOwn(fvc::interpolate(rhoU, own_(), scheme("rhoU")));
    surfaceVectorField rhoUNei(fvc::interpolate(rhoU, nei_(), scheme("rhoU")));

    surfaceVectorField UOwn = rhoUOwn/rhoOwn_();
    surfaceVectorField UNei = rhoUNei/rhoNei_();

    surfaceScalarField rPsiOwn(fvc::interpolate(rPsi, own_(), scheme("rPsi")));
    surfaceScalarField rPsiNei(fvc::interpolate(rPsi, nei_(), scheme("rPsi")));
    
    surfaceScalarField pOwn = rhoOwn_()*rPsiOwn;
    surfaceScalarField pNei = rhoNei_()*rPsiNei;

    surfaceScalarField cOwn(fvc::interpolate(c, own_(), scheme("c")));
    surfaceScalarField cNei(fvc::interpolate(c, nei_(), scheme("c")));

    surfaceScalarField eOwn(fvc::interpolate(e, own_(), scheme("e")));
    surfaceScalarField eNei(fvc::interpolate(e, nei_(), scheme("e")));

    volScalarField p = rho * rPsi;

    preUpdate(p);
    forAll(UOwn, facei)
    {

        calculateFluxes
        (
            rhoOwn_()[facei], rhoNei_()[facei],
            UOwn[facei], UNei[facei],
            eOwn[facei], eNei[facei],
            pOwn[facei], pNei[facei],
            cOwn[facei], cNei[facei],
            mesh_.Sf()[facei],
            phi[facei],
            rhoPhi[facei],
            rhoUPhi[facei],
            rhoEPhi[facei],
            facei
        );
    }

    forAll(rho.boundaryField(), patchi)
    {
        forAll(rho.boundaryField()[patchi], facei)
        {

            calculateFluxes
            (
                rhoOwn_().boundaryField()[patchi][facei],
                rhoNei_().boundaryField()[patchi][facei],
                UOwn.boundaryField()[patchi][facei],
                UNei.boundaryField()[patchi][facei],
                eOwn.boundaryField()[patchi][facei],
                eNei.boundaryField()[patchi][facei],
                pOwn.boundaryField()[patchi][facei],
                pNei.boundaryField()[patchi][facei],
                cOwn.boundaryField()[patchi][facei],
                cNei.boundaryField()[patchi][facei],
                mesh_.Sf().boundaryField()[patchi][facei],
                phi.boundaryFieldRef()[patchi][facei],
                rhoPhi.boundaryFieldRef()[patchi][facei],
                rhoUPhi.boundaryFieldRef()[patchi][facei],
                rhoEPhi.boundaryFieldRef()[patchi][facei],
                facei, patchi
            );
        }
    }
    postUpdate();
}

bool Foam::fluxScheme::writeData(Ostream& os) const
{
    return os.good();
}

// ************************************************************************* //
