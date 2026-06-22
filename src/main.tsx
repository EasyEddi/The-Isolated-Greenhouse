import { StrictMode, useMemo, useState } from "react";
import { createRoot } from "react-dom/client";
import "./styles.css";

type PlantStage = "small" | "growing" | "ready";

type Order = {
  id: number;
  title: string;
  requestedOffshoots: number;
  reward: number;
};

const orders: Order[] = [
  {
    id: 1,
    title: "First Monstera offshoot",
    requestedOffshoots: 1,
    reward: 35,
  },
  {
    id: 2,
    title: "Two healthy cuttings",
    requestedOffshoots: 2,
    reward: 80,
  },
  {
    id: 3,
    title: "Starter set for a bright room",
    requestedOffshoots: 3,
    reward: 140,
  },
];

function App() {
  const [plantStage, setPlantStage] = useState<PlantStage>("small");
  const [carePoints, setCarePoints] = useState(0);
  const [offshoots, setOffshoots] = useState(0);
  const [money, setMoney] = useState(20);
  const [orderIndex, setOrderIndex] = useState(0);
  const [fertilizer, setFertilizer] = useState(0);

  const activeOrder = orders[orderIndex];
  const isComplete = !activeOrder;

  const stageLabel = useMemo(() => {
    if (plantStage === "small") return "Young mother plant";
    if (plantStage === "growing") return "Growing steadily";
    return "Ready for cuttings";
  }, [plantStage]);

  function careForPlant() {
    const boost = fertilizer > 0 ? 2 : 1;
    const nextCare = carePoints + boost;

    if (fertilizer > 0) {
      setFertilizer((current) => current - 1);
    }

    setCarePoints(nextCare);

    if (nextCare >= 6) {
      setPlantStage("ready");
      return;
    }

    if (nextCare >= 3) {
      setPlantStage("growing");
    }
  }

  function cutOffshoot() {
    if (plantStage !== "ready") return;
    setOffshoots((current) => current + 1);
    setPlantStage("growing");
    setCarePoints(3);
  }

  function shipOrder() {
    if (!activeOrder || offshoots < activeOrder.requestedOffshoots) return;

    setOffshoots((current) => current - activeOrder.requestedOffshoots);
    setMoney((current) => current + activeOrder.reward);
    setOrderIndex((current) => current + 1);
  }

  function buyFertilizer() {
    if (money < 25) return;
    setMoney((current) => current - 25);
    setFertilizer((current) => current + 1);
  }

  return (
    <main className="game-shell">
      <section className="greenhouse" aria-label="Greenhouse">
        <div className="sunbeam" />
        <div className="roof">
          <span />
          <span />
          <span />
          <span />
        </div>

        <div className="hanging-plants">
          <span />
          <span />
          <span />
        </div>

        <div className={`plant plant-${plantStage}`} aria-label={stageLabel}>
          <div className="leaf leaf-one" />
          <div className="leaf leaf-two" />
          <div className="leaf leaf-three" />
          <div className="leaf leaf-four" />
          <div className="stem" />
          <div className="pot" />
        </div>

        <div className="workbench">
          <div className="tool" />
          <div className="soil-bag" />
          <div className="package-box" />
        </div>
      </section>

      <aside className="control-panel" aria-label="Greenhouse computer">
        <header>
          <p className="eyebrow">The Isolated Greenhouse</p>
          <h1>Monstera Orders</h1>
        </header>

        <section className="status-grid">
          <div>
            <span>Money</span>
            <strong>{money} credits</strong>
          </div>
          <div>
            <span>Cuttings</span>
            <strong>{offshoots}</strong>
          </div>
          <div>
            <span>Plant</span>
            <strong>{stageLabel}</strong>
          </div>
          <div>
            <span>Fertilizer</span>
            <strong>{fertilizer}</strong>
          </div>
        </section>

        <section className="order-panel">
          <h2>Current Order</h2>
          {isComplete ? (
            <p className="complete-message">
              All MVP orders are complete. The greenhouse is ready for the Friday demo.
            </p>
          ) : (
            <>
              <p className="order-title">{activeOrder.title}</p>
              <p>
                Needs {activeOrder.requestedOffshoots} Monstera cutting
                {activeOrder.requestedOffshoots > 1 ? "s" : ""}. Reward: {activeOrder.reward} credits.
              </p>
            </>
          )}
        </section>

        <section className="actions" aria-label="Actions">
          <button type="button" onClick={careForPlant}>
            Care for plant
          </button>
          <button type="button" onClick={cutOffshoot} disabled={plantStage !== "ready"}>
            Cut offshoot
          </button>
          <button
            type="button"
            onClick={shipOrder}
            disabled={!activeOrder || offshoots < activeOrder.requestedOffshoots}
          >
            Package order
          </button>
          <button type="button" onClick={buyFertilizer} disabled={money < 25}>
            Buy fertilizer
          </button>
        </section>
      </aside>
    </main>
  );
}

createRoot(document.getElementById("root")!).render(
  <StrictMode>
    <App />
  </StrictMode>,
);
