# Archicad bSDD Add-on

Experimental Archicad add-on integrating the buildingSMART Data Dictionary (bSDD).

The goal of this project is to develop a workflow for searching bSDD classifications and assigning dictionary-based properties directly to BIM elements in Archicad, with future support for IFC export and semantic interoperability.

## Project status

Early prototype (MVP stage).

The current focus is on validating the workflow:

1. Search for classes in bSDD
2. Select a classification
3. Retrieve class properties
4. Assign selected properties to Archicad elements
5. Store dictionary and class identifiers for IFC export

## Motivation

bSDD provides a standardized way to describe building objects and their properties.  
However, integration of bSDD in BIM authoring tools is still limited.

This project explores how bSDD can be integrated into Archicad workflows through a custom add-on.

## Planned functionality

MVP:

- connect to bSDD API
- search dictionaries and classes
- retrieve class properties
- assign selected properties to elements

Future development:

- automated property creation
- classification assignment
- IFC classification mapping
- IDS validation workflows

## Technology stack

- Archicad 29 Add-On API
- C++
- bSDD REST API
- IFC classification mapping

## Repository structure

- docs/ architecture notes and research
- src/ add-on source code
- samples/ example API responses
- test-data/ testing files

## Development methodology

This project follows an experimental **AI-assisted development workflow** (sometimes referred to as *vibe coding*), where software design, prototyping and parts of the implementation are developed interactively with the support of a large language model.

The primary AI system used during development is:

- **ChatGPT based on the GPT-5.3 architecture (OpenAI)**

The AI is used to assist with:

- architectural design of the add-on
- exploration of Archicad API workflows
- interpretation of the buildingSMART Data Dictionary (bSDD) API
- drafting example code and development patterns
- documentation and development planning

All design decisions, testing and final implementation choices are reviewed and validated by the repository maintainer.

AI-generated code and suggestions should therefore be treated as **experimental prototypes** and reviewed before use in production environments.

## References

buildingSMART Data Dictionary  
https://technical.buildingsmart.org/services/bsdd/

Archicad Developer Hub  
https://archicadapi.graphisoft.com/

---

Experimental research project.
