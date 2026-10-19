import { redirect } from "next/navigation";

/** Legacy intake → demo assigned job. */
export default function IntakeRedirect() {
  redirect("/jobs/seed-riverside-b03");
}
