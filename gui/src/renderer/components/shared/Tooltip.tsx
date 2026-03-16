import React, { useState, useRef } from 'react';
import ReactDOM from 'react-dom';

export function Tooltip({
  text,
  children,
  delay = 1500,
}: {
  text: string;
  children: React.ReactNode;
  delay?: number;
}) {
  const [pos, setPos] = useState<{ top: number; left: number } | null>(null);
  const timer = useRef<ReturnType<typeof setTimeout> | null>(null);
  const triggerRef = useRef<HTMLSpanElement>(null);

  function handleEnter() {
    timer.current = setTimeout(() => {
      if (triggerRef.current) {
        const rect = triggerRef.current.getBoundingClientRect();
        setPos({ top: rect.top, left: rect.left });
      }
    }, delay);
  }

  function handleLeave() {
    if (timer.current) clearTimeout(timer.current);
    setPos(null);
  }

  return (
    <span
      ref={triggerRef}
      className="inline-block"
      onMouseEnter={handleEnter}
      onMouseLeave={handleLeave}
    >
      {children}
      {pos && ReactDOM.createPortal(
        <span
          style={{
            position: 'fixed',
            top: pos.top - 6,
            left: pos.left,
            transform: 'translateY(-100%)',
            zIndex: 9999,
          }}
          className="bg-elevated border border-border rounded-input p-2 text-xs text-secondary shadow-lg pointer-events-none max-w-xs whitespace-normal block"
        >
          {text}
        </span>,
        document.body,
      )}
    </span>
  );
}
