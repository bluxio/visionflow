import { Suspense } from "react";

import { AssignedJob } from "@/components/jobs/assigned-job";

type PageProps = { params: Promise<{ id: string }> };

export default async function JobPage({ params }: PageProps) {
  const { id } = await params;
  return (
    <Suspense
      fallback={
        <div className="p-8 text-sm text-muted-foreground">Loading job…</div>
      }
    >
      <AssignedJob id={id} />
    </Suspense>
  );
}
