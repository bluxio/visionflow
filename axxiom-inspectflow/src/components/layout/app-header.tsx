import Link from "next/link";

import { buttonVariants } from "@/components/ui/button";
import { cn } from "@/lib/utils";
import { PAGE_GUTTER } from "@/lib/layout";

const nav = [
  { href: "/jobs/seed-riverside-b03", label: "Assigned job" },
  { href: "/dashboard", label: "Operations" },
];

export function AppHeader({ className }: { className?: string }) {
  return (
    <header
      className={cn(
        "sticky top-0 z-40 border-b border-border bg-background/95 backdrop-blur-sm",
        className,
      )}
    >
      <div
        className={cn(
          PAGE_GUTTER,
          "flex items-center justify-between gap-4 py-2.5",
        )}
      >
        <Link href="/jobs/seed-riverside-b03" className="min-w-0 space-y-0.5">
          <div className="flex items-baseline gap-2 text-[15px] tracking-tight">
            <span className="font-semibold text-navy">AXXIOM</span>
            <span className="text-border">/</span>
            <span className="font-medium text-foreground">InspectFlow</span>
          </div>
          <p className="text-[11px] font-medium tracking-wide text-muted-foreground">
            From field inspection to complete follow-through
          </p>
        </Link>
        <nav className="flex items-center gap-1 text-[13px] font-medium">
          {nav.map((item) => (
            <Link
              key={item.href}
              href={item.href}
              className={cn(
                buttonVariants({ variant: "ghost", size: "sm" }),
                "rounded-md text-muted-foreground hover:text-foreground",
              )}
            >
              {item.label}
            </Link>
          ))}
        </nav>
      </div>
    </header>
  );
}
