// Drop-in project card for the Next.js + Tailwind portfolio.
// 1. Copy to components/projects/RobotCarProjectCard.tsx
// 2. Replace YOUR-USERNAME (twice)
// 3. Render <RobotCarProjectCard /> in your Projects section
// When real photos exist, put one in public/projects/robot-car.jpg and
// set `image` below; until then the card shows no photo.
import { Github, ExternalLink, Cpu } from "lucide-react";

const project = {
  title: "4WD Robot Car",
  tag: "Project 04 · Robotics",
  status: "Firmware + simulation complete · hardware build in progress",
  summary:
    "Arduino UNO robot with line following, ultrasonic obstacle avoidance, Bluetooth phone control and joystick control. Modular C++ firmware with a single motor owner, failsafes, and 92 automated logic tests against a mocked Arduino core.",
  stack: ["Arduino C++", "ATmega328P", "L298N", "HC-SR04", "HC-05", "GitHub Actions"],
  repo: "https://github.com/YOUR-USERNAME/4WD-Robot-Car",
  page: "https://YOUR-USERNAME.github.io/4WD-Robot-Car/",
  image: null as string | null, // e.g. "/projects/robot-car.jpg" — real photos only
};

export default function RobotCarProjectCard() {
  return (
    <article className="rounded-xl border border-neutral-200 dark:border-neutral-800 p-6 flex flex-col gap-4">
      {project.image && (
        <img src={project.image} alt="Assembled 4WD robot car" className="rounded-lg aspect-[4/3] object-cover" />
      )}
      <div className="flex items-center gap-2 text-xs uppercase tracking-wider text-neutral-500">
        <Cpu size={14} /> {project.tag}
      </div>
      <h3 className="text-2xl font-bold">{project.title}</h3>
      <p className="text-sm text-amber-600 dark:text-amber-400">{project.status}</p>
      <p className="text-neutral-600 dark:text-neutral-300">{project.summary}</p>
      <ul className="flex flex-wrap gap-2">
        {project.stack.map((s) => (
          <li key={s} className="text-xs font-mono px-2 py-1 rounded bg-neutral-100 dark:bg-neutral-900">{s}</li>
        ))}
      </ul>
      <div className="flex gap-4 text-sm font-medium">
        <a href={project.page} target="_blank" rel="noreferrer" className="inline-flex items-center gap-1 text-teal-600 hover:underline">
          <ExternalLink size={16} /> Case study
        </a>
        <a href={project.repo} target="_blank" rel="noreferrer" className="inline-flex items-center gap-1 hover:underline">
          <Github size={16} /> Code
        </a>
      </div>
    </article>
  );
}
