import { ReviewWorkspace } from "@/components/review/review-workspace";

type PageProps = {
  params: Promise<{ id: string }>;
};

export default async function InspectionReviewPage({ params }: PageProps) {
  const { id } = await params;
  return <ReviewWorkspace id={id} />;
}
