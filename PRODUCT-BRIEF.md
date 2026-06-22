# Product Brief: The Isolated Greenhouse

## One-Sentence Pitch

The Isolated Greenhouse is a peaceful first-person plant simulator where the player grows mother plants, cuts offshoots, completes plant orders, and slowly fills an isolated greenhouse hall with a personal plant collection.

## Concept

The game takes place in one large, isolated, slightly overgrown greenhouse hall. The atmosphere should feel warm, calm, cozy, and peaceful. It should never feel scary or horror-like. The hall is lit by natural sunlight and warm lamps. Deliveries arrive through a roof hatch by drone.

The player is alone in the greenhouse. There are no NPCs, visible coworkers, or other humans. The focus is the quiet hobby feeling of caring for plants, discovering new species, and completing small goals.

## Target Experience

The game should feel like a relaxing hobby, not like a job. The player should always have a clear next goal, but the pacing should stay calm. Progress is shown through the growing collection of plants, tools, pots, soil, and greenhouse equipment.

## Core Loop

1. Check the active customer order on the greenhouse computer.
2. Grow or maintain the needed plant.
3. If required, build up a mother plant first.
4. Cut an offshoot from the mother plant.
5. Package and send the plant order.
6. Earn money.
7. Buy useful tools, pots, soil, fertilizer, or new plant options.
8. Unlock the next order.

Only one customer order is active at a time. After one order is completed, the next order unlocks automatically.

## Progression

Early orders are simple, such as one easy plant or one offshoot. Later orders can require specific sizes, quantities, or plant crossings. The difficulty should increase slowly and steadily.

The greenhouse hall stays roughly the same size. The feeling of progress comes from the plant collection and equipment, not from building expansions.

## Plants

Plants are mostly based on real species:

- Monstera
- Succulents
- Cacti
- Tropical plants

Later plants and crossings may be fictional, but they should still feel believable and natural.

## MVP Scope

The first playable version should include:

- A simple browser-based greenhouse view
- One plant species: Monstera
- A visible current order
- Basic plant state: small, growing, ready
- A button or interaction to care for the plant
- A way to cut and package an offshoot
- Order completion and money reward
- A small shop with at least one useful item
- A short order chain with 3 orders

## Out of Scope for MVP

These ideas are good for later, but not needed for the first week:

- Full 3D movement through a large greenhouse
- Many plant species
- Realistic plant genetics
- Login/accounts
- Multiplayer
- Complex database
- Large building expansion
- NPCs
- Horror or survival mechanics

## Technical Direction

Start as a web game that can be deployed to Vercel. The safest MVP path is a frontend-only app first. If saving progress becomes important, local storage can be used before adding a real backend.

Recommended stack:

- Vite
- React
- TypeScript
- CSS modules or plain CSS
- Vercel deployment

## Team Split

Eddi:

- Greenhouse main screen
- Plant display and plant states
- Plant care interaction
- Cutting offshoots

Tarek:

- Computer/order UI
- Shop UI
- Money/progression logic
- Order chain

Shared:

- Product decisions
- Pull request reviews
- Bug testing
- Demo preparation

## Success Criteria for Friday MVP

- The app is deployed on Vercel.
- The player can complete at least 3 orders.
- The player can grow/care for a Monstera and cut offshoots.
- Money increases after completing orders.
- There is a visible sense of greenhouse progress.
- Both team members have contributed through separate branches and pull requests.
